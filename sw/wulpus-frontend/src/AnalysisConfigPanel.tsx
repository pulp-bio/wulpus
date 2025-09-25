import type { UseMutationResult } from '@tanstack/react-query';
import { useState } from 'react';
import { NumberField, StringField } from "./Fields";
import { type AnalysisConfig, type ConnectResponse } from "./api";
import { toast } from 'react-hot-toast';

export function AnalysisConfigPanel(props: { analyzeConfig?: AnalysisConfig, setAnalyzeConfig: UseMutationResult<ConnectResponse, Error, AnalysisConfig, unknown> }) {
    const { analyzeConfig, setAnalyzeConfig } = props;
    const { spacers } = analyzeConfig ?? { spacers: [] };
    const [showAdvanced, setShowAdvanced] = useState(false);

    if (!analyzeConfig) {
        return (
            <div className="p-4 space-y-4">
                <h2 className="font-medium">Analysis Config</h2>
                <div className="p-4">Loading...</div>
            </div>
        );
    }
    else {
        const updateSpacer = (idx: number, field: 'thickness' | 'speedOfSound' | 'note', value: number | string) => {
            setAnalyzeConfig.mutate({
                ...analyzeConfig,
                spacers: analyzeConfig.spacers.map((s, i) => i === idx ? { ...s, [field]: value } : s)
            })
        }
        const addSpacer = () => {
            setAnalyzeConfig.mutate({
                ...analyzeConfig,
                spacers: [...analyzeConfig.spacers, { thickness: 1.5, speedOfSound: 2500, note: '' }]
            }, {
                onError: (error) => {
                    toast.error(`Failed to add spacer - Is there a server?\n${error.message}`);
                }
            });
        }
        const removeSpacer = (idx: number) => {
            setAnalyzeConfig.mutate({
                ...analyzeConfig,
                spacers: analyzeConfig.spacers.filter((_, i) => i !== idx)
            })
        };

        return (
            <div className="p-4 space-y-4">
                <div>
                    <div className='col-span-2 flex items-center justify-between'>
                        <h2 className="font-medium">Peak Detection</h2>
                        <button
                            type="button"
                            onClick={() => setShowAdvanced(s => !s)}
                            className="p-0.5 gap-1 rounded flex items-center text-gray-700 bg-white hover:bg-gray-50"
                            style={{ fontSize: 100, lineHeight: 1 }}
                            aria-label='Toggle advanced fields'
                        >
                            <span className="material-symbols-rounded">{showAdvanced ? 'visibility_off' : "visibility"}</span>
                        </button>
                    </div>
                    <div className="mb-1 text-gray-600 text-sm">Configure sections to be ignored</div>
                </div>
                <div className="space-y-3">
                    {spacers.map((sp, idx) => (
                        <div key={idx} className="border rounded p-3 bg-white space-y-2">
                            <div className="flex items-center justify-between">
                                <div className="font-medium text-sm">Ignore Layer #{idx + 1}</div>
                                <button
                                    type="button"
                                    onClick={() => removeSpacer(idx)}
                                    className="text-xs text-red-600 hover:underline"
                                >
                                    Remove
                                </button>
                            </div>
                            <div className="grid grid-cols-2 gap-3">
                                <NumberField label="Thickness [mm]" value={sp.thickness} step={0.1} onChange={(v) => updateSpacer(idx, 'thickness', v)} />
                                <NumberField label="c [m/s]" value={sp.speedOfSound} step={100} onChange={(v) => updateSpacer(idx, 'speedOfSound', v)} />
                            </div>
                            <div className="flex flex-col gap-1">
                                <StringField label="Note" value={sp.note ?? ''} onChange={(v) => updateSpacer(idx, 'note', v)} placeholder="optional" />
                            </div>
                        </div>
                    ))}
                    <div className='col-span-2 flex items-center justify-between'>
                        <button onClick={addSpacer} className="bg-gray-100 hover:bg-gray-200 rounded px-3 py-2 text-sm">Add layer</button>
                        {spacers.length === 0 && (
                            <div className="text-xs text-gray-500">No spacers defined. Add one.</div>
                        )}
                    </div>
                </div>
                <div className='grid grid-cols-2 gap-3' hidden={!showAdvanced}>
                    <NumberField label="Min Peak Consistency" value={analyzeConfig.peakConsistency} step={1} onChange={(v) => setAnalyzeConfig.mutate({ ...analyzeConfig, peakConsistency: v })} />
                    <NumberField label="Min Peak Threshold" value={analyzeConfig.peakThreshold} step={1} onChange={(v) => setAnalyzeConfig.mutate({ ...analyzeConfig, peakThreshold: v })} />
                    <NumberField label="Min Peak History" value={analyzeConfig.peakHistory} step={1} onChange={(v) => setAnalyzeConfig.mutate({ ...analyzeConfig, peakHistory: v })} />
                    <NumberField label="Max Peaks" value={analyzeConfig.nMaxPeaks} step={1} onChange={(v) => setAnalyzeConfig.mutate({ ...analyzeConfig, nMaxPeaks: v })} />
                    <NumberField label="Upsampling Factor" value={analyzeConfig.upsamplingFactor} step={1} onChange={(v) => setAnalyzeConfig.mutate({ ...analyzeConfig, upsamplingFactor: v })} />
                </div>
            </div>
        );
    }
}