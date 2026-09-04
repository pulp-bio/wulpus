import type Plotly from 'plotly.js';
import { useCallback, useEffect, useRef, useState } from "react";
import Plot from 'react-plotly.js';
import { bandpassFIR, hilbertEnvelope, toggleFullscreen } from './helper';
import type { DataFrame, UsConfig } from './websocket-types';
import RangeSlider from 'react-range-slider-input';

export function Graph(props: { dataFrame: DataFrame | undefined, bmodeBuffer: number[][], peaksPerChannel: number[][], usConfig: UsConfig }) {
    const { dataFrame, bmodeBuffer, peaksPerChannel, usConfig } = props;
    const data = dataFrame?.measurement.data ?? [];
    const wavelet_transform = dataFrame?.wavelet ?? [];
    const peaks = dataFrame?.peaks ?? [];
    const sampling_freq = usConfig.sampling_freq;
    const plotContainerRef = useRef<HTMLDivElement | null>(null);
    const [showBMode, setShowBMode] = useState<boolean>(false);
    const UPSAMPLING_FACTOR = 10

    // fullscreen graph support
    const [isFullscreen, setIsFullscreen] = useState(false);

    // track fullscreen changes
    useEffect(() => {
        const onFsChange = () => setIsFullscreen(Boolean(document.fullscreenElement));
        document.addEventListener('fullscreenchange', onFsChange);
        return () => document.removeEventListener('fullscreenchange', onFsChange);
    }, []);

    // compute filter/envelope just-in-time before rendering
    const minLowCutHz = useCallback((sampling_freq: number) => sampling_freq / 2 * 0.1, []);
    const maxHighCutHz = useCallback((sampling_freq: number) => sampling_freq / 2 * 0.9, []);
    const [lowCutHz, setLowCutHz] = useState(minLowCutHz(sampling_freq));
    const [highCutHz, setHighCutHz] = useState(maxHighCutHz(sampling_freq));
    const filteredFrame = data ? bandpassFIR(data, sampling_freq, lowCutHz, highCutHz, 31) : [];
    const envelopeFrame = filteredFrame.length ? hilbertEnvelope(filteredFrame, 101) : [];

    useEffect(() => {
        setLowCutHz(minLowCutHz(sampling_freq));
        setHighCutHz(maxHighCutHz(sampling_freq));
    }, [sampling_freq, setHighCutHz, minLowCutHz, maxHighCutHz]);

    // Rx channels for current frame (if provided)

    // Vertical line shapes for the time-domain (non B-mode) plot spanning full height
    const signalPeakShapes: Partial<Plotly.Shape>[] = peaks.map(p => ({
        type: 'line', x0: p, x1: p, xref: 'x', yref: 'paper', y0: 0, y1: 1,
        line: { color: 'rgba(255,140,0,0.35)', width: 2, dash: 'dot' }, layer: 'below'
    }));

    const spacerShape: Partial<Plotly.Shape> = {
        type: 'rect', x0: 0, x1: dataFrame?.spacer_region[1] ?? 0, xref: 'x', yref: 'paper', y0: 0, y1: 1,
        line: { color: 'rgba(100,120,135,0.35)' }, fillcolor: 'rgba(100,120,135,0.35)', layer: 'below'
    }

    // For B-Mode heatmap: draw peak lines only over the rows (channels) that belong to this measurement's rx set.
    // Heatmap implicit y coordinates: row indices 0..N-1. We'll span each channel row from (ch-0.5) to (ch+0.5)
    const bmodePeakShapes: Partial<Plotly.Shape>[] = [];
    if (peaksPerChannel && peaksPerChannel.length) {
        for (let ch = 0; ch < peaksPerChannel.length; ch++) {
            const chPeaks = peaksPerChannel[ch];
            if (!chPeaks || !chPeaks.length) continue;
            if (ch < 0 || ch >= bmodeBuffer.length) continue;
            for (const p of chPeaks) {
                bmodePeakShapes.push({
                    type: 'line',
                    x0: p, x1: p,
                    xref: 'x', yref: 'y',
                    y0: ch - 0.5, y1: ch + 0.5,
                    line: { color: 'rgba(255,140,0,0.55)', width: 2, dash: 'dot' },
                    layer: 'above'
                });
            }
        }
    }

    return (
        <div ref={plotContainerRef} className="bg-white p-4">
            <div className="h-[400px]">
                {showBMode ? (
                    <Plot
                        data={[{
                            z: bmodeBuffer.length ? bmodeBuffer : [[]],
                            type: 'heatmap',
                            colorscale: 'Viridis',
                            reversescale: true,
                        }] as Plotly.Data[]}
                        useResizeHandler
                        style={{ width: "100%", height: "100%" }}
                        layout={{
                            autosize: true,
                            margin: { t: 10, r: 10, b: 30, l: 40 },
                            shapes: [...bmodePeakShapes, spacerShape],
                            yaxis: { autorange: true, title: { text: 'Channel' } },
                        }}
                    />
                ) : (
                    <Plot
                        data={([
                            {
                                x: data ? data.map((_, i) => i) : [],
                                y: data ?? [],
                                type: 'scatter', mode: 'lines', name: 'Raw', line: { color: 'blue' },
                            },
                            {
                                    x: data ? data.map((_, i) => i) : [],
                                    y: filteredFrame.length ? filteredFrame : [],
                                    type: 'scatter', mode: 'lines', name: 'Filter', line: { color: 'green' },
                                    visible: 'legendonly',
                                },
                                {
                                    x: data ? data.map((_, i) => i) : [],
                                    y: envelopeFrame.length ? envelopeFrame : [],
                                    type: 'scatter', mode: 'lines', name: 'Envelope', line: { color: 'fuchsia' },
                                    visible: 'legendonly',
                                },
                                {
                                    x: wavelet_transform ? wavelet_transform.map((_, i) => i / UPSAMPLING_FACTOR) : [],
                                    y: wavelet_transform ?? [],
                                    type: 'scatter', mode: 'lines', name: 'Wavelet Envelope', line: { color: 'red' },
                                visible: 'legendonly',
                                }
                            ]) as Plotly.Data[]}
                        useResizeHandler
                        style={{ width: "100%", height: "100%" }}
                        layout={{
                            autosize: true,
                            uirevision: "fixed",
                            showlegend: true,
                            legend: { orientation: 'h' },
                            margin: { t: 10, r: 10, b: 30, l: 40 },
                            yaxis: { range: [-2000, 2000] },
                            shapes: [...signalPeakShapes, spacerShape],
                        }}
                    />
                )}
            </div>
            <div className="flex gap-2 mt-2 flex-wrap items-center">
                <button
                    onClick={() => setShowBMode(o => !o)}
                    className={`bg-gray-500 hover:bg-gray-600 text-white rounded p-1`}
                >
                    {showBMode ? 'Disable' : 'Enable'} B Mode</button>
                <button
                    onClick={() => toggleFullscreen(plotContainerRef)}
                    className={`border-gray-500 border-1 hover:bg-gray-200 rounded p-1 flex items-center justify-center`}
                >
                    {isFullscreen ? <span className="material-symbols-rounded">fullscreen_exit</span> : <span className="material-symbols-rounded">fullscreen</span>}
                </button>

                {!showBMode && (
                    <>
                    <div className="flex grow items-center ml-4 gap-3">
                        <span>Filter: </span>
                        <div className='flex grow max-w-96 items-center justify-start gap-2'>
                            <span className='w-32'>{Math.round(lowCutHz / 1e4) / 100} MHz</span>
                            <div className='w-full'>
                                <RangeSlider
                                    min={minLowCutHz(sampling_freq)}
                                    max={maxHighCutHz(sampling_freq)}
                                    step={sampling_freq / 1e4}
                                    value={[lowCutHz, highCutHz]}
                                    onInput={i => {
                                        const [low, high] = i;
                                        setLowCutHz(low);
                                        setHighCutHz(high);
                                    }}
                                />
                            </div>
                            <span className='w-32'>{Math.round(highCutHz / 1e4) / 100} MHz</span>
                        </div>
                    </div>
                        <div className="items-center gap-2">
                            <span className='px-2 py-0.5 text-xs rounded-md border border-gray-200 bg-white'>
                                {dataFrame?.measurement.rx && dataFrame.measurement.rx.length > 0 ? `Rx: ${dataFrame.measurement.rx.join(', ')}` : 'No Signal'}
                            </span>
                        </div>
                    </>
                )}
            </div>
        </div>)
}