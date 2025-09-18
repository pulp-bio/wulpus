import { useCallback, useEffect, useMemo, useState } from 'react';
import { toast } from 'react-hot-toast';
import useWebSocket from 'react-use-websocket';
import { ConnectionPanel } from './ConnectionPanel';
import { Graph } from './Graph';
import { TxRxConfigPanel } from './TxRxConfig';
import { USConfigPanel } from './UsConfig';
import { ConfigFilesPanel } from './ConfigFilesPanel';
import { SeriesPanel } from './SeriesPanel';
import type { DataFrame, Status, TxRxConfig, UsConfig, WulpusConfig } from './websocket-types';
import { getInitialConfig } from './helper';

export const LOCAL_KEY = 'wulpus-config-v1';
export const CHANNEL_SIZE = 8;

function App() {

  const wsUrl = `${window.location.protocol === 'https:' ? 'wss' : 'ws'}://${window.location.host}/ws`;
  const { lastJsonMessage } = useWebSocket<Status | DataFrame>(wsUrl, {
    shouldReconnect: () => true,
  });

  const [status, setStatus] = useState<Status | null>(null);
  const [dataFrame, setDataFrame] = useState<DataFrame | null>(null);

  const [bmodeBuffer, setBmodeBuffer] = useState<number[][]>(Array.from({ length: CHANNEL_SIZE }, () => []));

  // WulpusConfig state
  const [txRxConfigs, setTxRxConfigs] = useState<TxRxConfig[]>(getInitialConfig().tx_rx_config);
  const [usConfig, setUsConfig] = useState<UsConfig>(getInitialConfig().us_config);

  const effectiveConfig: WulpusConfig = useMemo(() => ({
    tx_rx_config: txRxConfigs,
    us_config: {
      ...usConfig, num_txrx_configs: txRxConfigs.length,
      // ensure tx/rx bitmasks lists align in length
      tx_configs: txRxConfigs.map((c) => c.config_id),
      rx_configs: txRxConfigs.map((c) => c.config_id),
    },
  }), [txRxConfigs, usConfig]);

  const saveConfigToLocalStorage = useCallback((config: WulpusConfig) => {
    try {
      localStorage.setItem(LOCAL_KEY, JSON.stringify(config));
    } catch (e) {
      toast.error('Failed to store config in browser (localStorage)');
    }
  }, []);

  useEffect(() => {
    if (effectiveConfig) {
      saveConfigToLocalStorage(effectiveConfig)
    }
  }, [effectiveConfig, saveConfigToLocalStorage])


  useEffect(() => {
    if (lastJsonMessage) {
      if ('status' in lastJsonMessage) {
        setStatus(lastJsonMessage);
      }
      else if ('data' in lastJsonMessage) {
        setDataFrame(lastJsonMessage);
        const rx_channel = lastJsonMessage.rx;
        // push into bmode buffer
        setBmodeBuffer((prev) => {
          const next = [...prev];
          const new_data = lastJsonMessage.data.slice()
          for (const channel of rx_channel) {
            if (channel >= CHANNEL_SIZE) break;
            next[channel] = new_data;
          }
          return next;
        });
      }
    }
  }, [lastJsonMessage, setStatus, setDataFrame]);

  return (
    <div className="min-h-screen bg-gray-50 text-gray-900">
      <div className="border-b bg-white">
        <div className="mx-auto max-w-7xl px-4 sm:px-6 lg:px-8 py-2 flex items-center justify-between">
          <h1 className="text-lg font-semibold">Wulpus Dashboard</h1>
          <a href="/data" className="text-sm text-blue-600 hover:underline">Recorded logs</a>
        </div>
      </div>
      <main className="mx-auto max-w-7xl px-4 sm:px-6 lg:px-8 py-3 grid grid-cols-1 lg:grid-cols-3 gap-3">
        <section className="lg:col-span-1 space-y-3">
          <div className="bg-white rounded-lg shadow">
            <ConnectionPanel effectiveConfig={effectiveConfig} status={status} />
          </div>

          <div className="bg-white rounded-lg shadow">
            <SeriesPanel effectiveConfig={effectiveConfig} disabled={(status?.status ?? 0) !== 2} seriesStatus={status?.series} />
          </div>

          <div className="bg-white rounded-lg shadow">
            <USConfigPanel usConfig={usConfig} setUsConfig={setUsConfig} />
          </div>
        </section>

        <div className="col-span-2 space-y-3">
          <div className="bg-white rounded-lg shadow">
            <Graph dataFrame={dataFrame} bmodeBuffer={bmodeBuffer} usConfig={usConfig} />
          </div>

          <div className="bg-white rounded-lg shadow">
            <TxRxConfigPanel txRxConfigs={txRxConfigs} setTxRxConfigs={setTxRxConfigs} />
          </div>

          <div className="bg-white rounded-lg shadow">
            <ConfigFilesPanel
              effectiveConfig={effectiveConfig}
              applyConfig={(conf) => {
                if (conf?.tx_rx_config) setTxRxConfigs(conf.tx_rx_config);
                if (conf?.us_config) setUsConfig(conf.us_config);
              }}
            />
          </div>
        </div>
      </main>
    </div>
  )
}

export default App