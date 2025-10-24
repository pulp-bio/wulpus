import { useState } from 'react';
import { NumberField, SelectField } from "./Fields";
import type { UsConfig } from './websocket-types';

export function USConfigPanel(props: { usConfig: UsConfig, setUsConfig: React.Dispatch<React.SetStateAction<UsConfig>> }) {
    const { usConfig, setUsConfig } = props;
    const [showAdvanced, setShowAdvanced] = useState(false);

    const advancedFields = (
        <>
            <NumberField label="DCDC turn on (us)" value={usConfig.dcdc_turnon} onChange={(v) => setUsConfig(s => ({ ...s, dcdc_turnon: v }))} />
            <NumberField label="Trans freq (Hz)" value={usConfig.trans_freq} onChange={(v) => setUsConfig(s => ({ ...s, trans_freq: v }))} />
            <NumberField label="Pulse freq (Hz)" value={usConfig.pulse_freq} onChange={(v) => setUsConfig(s => ({ ...s, pulse_freq: v }))} />
            <SelectField label="Sampling freq"
                value={usConfig.sampling_freq}
                onChange={(v) => setUsConfig(s => ({ ...s, sampling_freq: Number(v) as UsConfig['sampling_freq'] }))}
                options={[8000000, 4000000, 2000000, 1000000, 500000].map(v => ({ value: String(v), label: String(v / 1000000) + "MHz" }))}
            />
            <SelectField label="RX gain (dB)"
                value={usConfig.rx_gain}
                onChange={(v) => setUsConfig(s => ({ ...s, rx_gain: parseFloat(v) }))}
                options={[-6.5, -5.5, -4.6, -4.1, -3.3, -2.3, -1.4, -0.8,
                    0.1, 1.0, 1.9, 2.6, 3.5, 4.4, 5.2, 6.0, 6.8, 7.7,
                    8.7, 9.0, 9.8, 10.7, 11.7, 12.2, 13, 13.9, 14.9,
                    15.5, 16.3, 17.2, 18.2, 18.8, 19.6, 20.5, 21.5,
                    22, 22.8, 23.6, 24.6, 25.0, 25.8, 26.7, 27.7,
                    28.1, 28.9, 29.8, 30.8].map(v => ({ value: String(v), label: String(v) }))}
            />
            <NumberField label="Start HV-MUX RX (us)" value={usConfig.start_hvmuxrx} onChange={(v) => setUsConfig(s => ({ ...s, start_hvmuxrx: v }))} />
            <NumberField label="Start PPG (us)" value={usConfig.start_ppg} onChange={(v) => setUsConfig(s => ({ ...s, start_ppg: v }))} />
            <NumberField label="Turn on ADC (us)" value={usConfig.turnon_adc} onChange={(v) => setUsConfig(s => ({ ...s, turnon_adc: v }))} />
            <NumberField label="Start PGA bias (us)" value={usConfig.start_pgainbias} onChange={(v) => setUsConfig(s => ({ ...s, start_pgainbias: v }))} />
            <NumberField label="Start ADC sample (us)" value={usConfig.start_adcsampl} onChange={(v) => setUsConfig(s => ({ ...s, start_adcsampl: v }))} />
            <NumberField label="Restart capture (us)" value={usConfig.restart_capt} onChange={(v) => setUsConfig(s => ({ ...s, restart_capt: v }))} />
            <NumberField label="Capture timeout (us)" value={usConfig.capt_timeout} onChange={(v) => setUsConfig(s => ({ ...s, capt_timeout: v }))} />
        </>
    );

    return (
        <div className="p-4 space-y-3">
            <div className="flex items-center justify-between">
                <h2 className="font-medium">Measurement Config</h2>
                <button
                    type="button"
                    onClick={() => setShowAdvanced(s => !s)}
                    className="p-0.5 gap-1 rounded flex items-center text-gray-700 bg-white hover:bg-gray-50"
                    style={{ fontSize: 100, lineHeight: 1 }}
                    aria-label='Toggle advanced fields'
                >
                    <span className="material-symbols-rounded">{showAdvanced ? "unfold_less" : "unfold_more"}</span>
                </button>
            </div>
            <div className={"grid gap-3 " + (showAdvanced ? 'grid-cols-2' : 'grid-cols-2 md:grid-cols-2')}> {/* Keep two columns either way */}
                <NumberField label="Num acquisitions" value={usConfig.num_acqs} onChange={(v) => setUsConfig(s => ({ ...s, num_acqs: v }))} />
                <NumberField label="Num samples" value={usConfig.num_samples} onChange={(v) => setUsConfig(s => ({ ...s, num_samples: v }))} />
                <NumberField label="Meas period (us)" value={usConfig.meas_period} onChange={(v) => setUsConfig(s => ({ ...s, meas_period: v }))} />
                <NumberField label="# Pulses" value={usConfig.num_pulses} onChange={(v) => setUsConfig(s => ({ ...s, num_pulses: v }))} />
                {showAdvanced && advancedFields}
            </div>
        </div>
    );
}