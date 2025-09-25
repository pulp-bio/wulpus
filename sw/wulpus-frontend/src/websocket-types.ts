// Types mirrored from backend pydantic models

export type UsConfig = {
    num_acqs: number;
    dcdc_turnon: number;
    meas_period: number;
    trans_freq: number;
    pulse_freq: number;
    num_pulses: number;
    sampling_freq: 8000000 | 4000000 | 2000000 | 1000000 | 500000;
    num_samples: number;
    rx_gain: number; // constrained to predefined values on backend
    num_txrx_configs: number;
    tx_configs: number[];
    rx_configs: number[];
    start_hvmuxrx: number;
    start_ppg: number;
    turnon_adc: number;
    start_pgainbias: number;
    start_adcsampl: number;
    restart_capt: number;
    capt_timeout: number;
};

export type TxRxConfig = {
    config_id: number;
    tx_channels: number[];
    rx_channels: number[];
    optimized_switching: boolean;
};

export type WulpusConfig = {
    tx_rx_config: TxRxConfig[];
    us_config: UsConfig;
};

export type SeriesStatus = {
    active: boolean;
    interval_seconds?: number;
    number?: number;
    progress_count?: number;
}

export type Status = {
    mock?: boolean;
    status: number; // 0.., maps to backend Status enum
    bluetooth: string;
    us_config: UsConfig | null;
    tx_rx_config: TxRxConfig[] | null;
    progress: number; // 0..1
    series?: SeriesStatus
};

export type DataFrame = {
    measurement: {
        data: number[]
        time: number[]
        tx: number[]
        rx: number[]
    }
    peaks: number[]
    wavelet: number[]
    spacer_region: [number, number]
}