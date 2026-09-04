import { CHANNEL_SIZE } from "./App";
import { MultiNumField } from "./MultiNumField";
import type { TxRxConfig } from './websocket-types';

export function TxRxConfigPanel(props: { txRxConfigs: TxRxConfig[], setTxRxConfigs: React.Dispatch<React.SetStateAction<TxRxConfig[]>> }) {
    const { txRxConfigs, setTxRxConfigs } = props;

    function updateTxRx<K extends keyof TxRxConfig>(idx: number, field: K, value: TxRxConfig[K]) {
        setTxRxConfigs((prev) => prev.map((c, i) => i === idx ? { ...c, [field]: value } : c));
    }

    function addTxRx() {
        setTxRxConfigs((prev) => [...prev, { config_id: prev.length, tx_channels: [], rx_channels: [], optimized_switching: true }]);
    }

    function removeTxRx(idx: number) {
        setTxRxConfigs((prev) => prev.filter((_, i) => i !== idx).map((c, i) => ({ ...c, config_id: i })));
    }


    return (
        <div className="p-4 space-y-3">
            <h2 className="font-medium">TX/RX Configurations</h2>
            <div className="space-y-4">
                {txRxConfigs.map((cfg, idx) => (
                    <div key={idx} className="border rounded p-3">
                        <div className="flex items-center justify-between mb-2">
                            <div className="font-medium">Config #{cfg.config_id}</div>
                            <button onClick={() => removeTxRx(idx)} className="text-red-600 text-sm">Remove</button>
                        </div>
                        <div className="grid grid-cols-3 gap-3">
                            <MultiNumField label={`TX channels (0-${CHANNEL_SIZE - 1})`}
                                values={cfg.tx_channels}
                                onChange={(vals) => updateTxRx(idx, 'tx_channels', vals)}
                                showChannelBoxes={true}
                                color="bg-green-500" />
                            <MultiNumField label={`RX channels (0-${CHANNEL_SIZE - 1})`}
                                values={cfg.rx_channels}
                                onChange={(vals) => updateTxRx(idx, 'rx_channels', vals)}
                                showChannelBoxes={true}
                                color="bg-blue-500" />
                            <div className="col-span-3 flex items-center gap-2">
                                <input
                                    id={`opt-${idx}`}
                                    type="checkbox"
                                    className="h-4 w-4"
                                    checked={cfg.optimized_switching}
                                    onChange={(e) => updateTxRx(idx, 'optimized_switching', e.target.checked)}
                                />
                                <label
                                    htmlFor={`opt-${idx}`}
                                    className="text-sm">Optimized switching
                                </label>
                            </div>
                        </div>
                    </div>
                ))}
                {txRxConfigs.length < CHANNEL_SIZE && (
                    <button onClick={addTxRx} className="bg-gray-100 hover:bg-gray-200 rounded px-3 py-2 text-sm">Add configuration</button>
                )}
            </div>
        </div>
    )
}