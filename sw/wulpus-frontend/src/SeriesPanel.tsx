import { useState } from 'react';
import { startSeries, stopSeries } from './api';
import type { SeriesStatus, WulpusConfig } from './websocket-types';
import { toast } from 'react-hot-toast';
import { NumberField } from './Fields';

export function SeriesPanel(props: { effectiveConfig: WulpusConfig, disabled?: boolean, seriesStatus?: SeriesStatus }) {
    const { effectiveConfig, disabled, seriesStatus } = props;
    const [intervalMinutes, setIntervalMinutes] = useState<number>(5);
    const intervalSeconds = intervalMinutes * 60;
    const [nIntervals, setNIntervals] = useState<number>(10);
    const status: SeriesStatus = seriesStatus || { active: false };

    async function handleStart() {
        try {
            const est = (effectiveConfig.us_config.num_acqs * effectiveConfig.us_config.meas_period) / 1e6;
            if (est >= intervalSeconds) {
                toast.error(`Estimated duration ${(est).toFixed(2)}s >= interval ${intervalSeconds}s`);
                return;
            }
            await startSeries(intervalSeconds, effectiveConfig, nIntervals);
            toast.success('Series started');
        } catch (e: unknown) {
            const msg = e instanceof Error ? e.message : String(e);
            toast.error(msg);
        }
    }

    async function handleStop() {
        try {
            await stopSeries();
            toast('Series stopped');
        } catch (e: unknown) {
            const msg = e instanceof Error ? e.message : String(e);
            toast.error(msg);
        }
    }

    return (
        <div className="p-4 space-y-2">
            <div className="flex gap-2 flex-row items-center">
                <h2 className="font-medium grow">Measurement Series</h2>
                {seriesStatus?.active &&
                    <>
                        <button
                            onClick={handleStop}
                            className="font-medium text-red-500 border-1 px-2 border-red-500 hover:bg-gray-50 rounded"
                        >
                            Series running
                        </button>
                    </>
                }
            </div>


            <div className="grid grid-cols-2 gap-3">
                <NumberField label="Interval (minutes)" value={intervalMinutes} onChange={(v) => setIntervalMinutes(v)} />
                <NumberField label="Num measurements" value={nIntervals} onChange={(v) => setNIntervals(v)} />
            </div>
            <div className='flex flex-row items-baseline gap-3'>
                <div>
                    {!status.active &&
                        <button onClick={handleStart} disabled={disabled}
                            className="ml-auto bg-indigo-600 hover:bg-indigo-700 text-white rounded px-3 py-2 disabled:opacity-50">
                            Start Series
                        </button>
                    }
                    {status.active &&
                        <button onClick={handleStop} className="ml-auto bg-red-600 hover:bg-red-700 text-white rounded px-3 py-2">
                            Stop Series
                        </button>
                    }
                </div>
                <div className="text-xs text-gray-600">Total Duration: {intervalMinutes * nIntervals}min</div>
            </div>
        </div>
    );
}
