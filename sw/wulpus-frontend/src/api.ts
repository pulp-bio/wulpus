// Simple API client for the FastAPI backend

import type { WulpusConfig } from "./websocket-types";

export type ConnectResponse = { ok: string } | { [key: string]: string };

export type ConnectionType = 'serial' | 'ble';
export type ConnectionOption = {
    device: string;           // e.g. COM5 or BLE MAC/address
    description: string;      // human-friendly label
    type: ConnectionType;     // 'serial' | 'ble'
};


export type AnalysisConfig = {
    spacers: { "thickness": number, "note"?: string, "speedOfSound": number }[]
    peakConsistency: number,
    peakThreshold: number,
    peakHistory: number,
    nMaxPeaks: number,
    upsamplingFactor: number,
};

// Use Vite proxy in dev to avoid CORS; see vite.config.ts
export const BASE_URL = "/api";

export async function getBTHConnections(): Promise<ConnectionOption[]> {
    const res = await fetch(`${BASE_URL}/connections`);
    if (!res.ok) throw new Error(`GET /connections failed: ${res.status}`);
    const data = await res.json();

    if (Array.isArray(data)) {
        // Handle new multi-device format: [{ wulpus_id?, options: [{ device, description, type }] }, ...]
        // or old format: [{ device, description, type }, ...]
        const allItems: ConnectionOption[] = [];

        for (const item of data) {
            if (item && typeof item === 'object') {
                // New format with options array
                if ('options' in item && Array.isArray(item.options)) {
                    const deviceId = item.wulpus_id ?? 0;
                    const devicePrefix = deviceId > 0 ? `[${deviceId}] ` : '';

                    for (const option of item.options) {
                        const obj = option as Partial<ConnectionOption> & Record<string, unknown>;
                        const t = obj.type === 'ble' || obj.type === 'serial' ? obj.type : 'serial';
                        allItems.push({
                            device: String(obj.device ?? ''),
                            description: devicePrefix + String(obj.description ?? obj.device ?? ''),
                            type: t,
                        } as ConnectionOption);
                    }
                } else {
                    // Old format - direct device object
                    const obj = item as Partial<ConnectionOption> & Record<string, unknown>;
                    const t = obj.type === 'ble' || obj.type === 'serial' ? obj.type : 'serial';
                    allItems.push({
                        device: String(obj.device ?? ''),
                        description: String(obj.description ?? obj.device ?? ''),
                        type: t,
                    } as ConnectionOption);
                }
            }
        }

        const uniqueItems = Array.from(new Map(allItems.map(i => [i.device, i])).values());
        return uniqueItems;
    }
    return [];
}

export async function postConnect(conDev: string): Promise<void> {
    const res = await fetch(`${BASE_URL}/connect`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ con_dev: conDev }),
    });
    if (!res.ok) throw new Error(`POST /connect failed: ${res.status}`);
}

export async function postDisconnect(conDev?: string): Promise<void> {
    let res: Response
    if (!conDev) {
        res = await fetch(`${BASE_URL}/disconnect/all`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
        });
    } else {
        res = await fetch(`${BASE_URL}/disconnect`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ con_dev: conDev }),
        });
    }
    if (!res.ok) throw new Error(`POST /disconnect failed: ${res.status}`);
}

export async function postStart(config: WulpusConfig): Promise<ConnectResponse> {
    const res = await fetch(`${BASE_URL}/start`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(config),
    });
    if (!res.ok) {
        let message = `${res.status}`;
        try {
            const errorData = await res.json();
            message += ` (${errorData.detail[0]?.msg ?? JSON.stringify(errorData)})`;
        }
        catch { /* empty */ }
        throw new Error(message)
    };
    return res.json();
}

export async function postStop(): Promise<ConnectResponse> {
    const res = await fetch(`${BASE_URL}/stop`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
    });
    if (!res.ok) throw new Error(`POST /stop failed: ${res.status}`);
    return res.json();
}

export async function postActivateMock(): Promise<void> {
    const res = await fetch(`${BASE_URL}/activate-mock`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
    });
    if (!res.ok) throw new Error(`POST /activate-mock failed: ${res.status}`);
}

export async function deactivateMock(): Promise<ConnectResponse> {
    const res = await fetch(`${BASE_URL}/deactivate-mock`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
    });
    if (!res.ok) throw new Error(`POST /deactivate-mock failed: ${res.status}`);
    return res.json();
}

export async function replayFile(filename: string): Promise<void> {
    const res = await fetch(`${BASE_URL}/replay/${encodeURIComponent(filename)}`, {
        method: 'POST',
        headers: { "Content-Type": "application/json" },
    });
    if (!res.ok) throw new Error(await res.text());
}

export async function getLogs(): Promise<string[]> {
    const res = await fetch(`${BASE_URL}/logs`);
    if (!res.ok) throw new Error(await res.text());
    return res.json();
}

export function StatusLabel(s?: number) {
    switch (s) {
        case 0: return 'NOT_CONNECTED';
        case 1: return 'CONNECTING';
        case 2: return 'READY';
        case 3: return 'RUNNING';
        case 9: return 'ERROR';
        default: return String(s ?? '—');
    }
}

export async function startSeries(intervalSeconds: number, config: WulpusConfig, number: number) {
    const res = await fetch(`${BASE_URL}/series/start`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            interval_seconds: intervalSeconds,
            config,
            number
        })
    });
    if (!res.ok) throw new Error(await res.text());
    return res.json();
}

export async function stopSeries() {
    const res = await fetch(`${BASE_URL}/series/stop`, { method: 'POST' });
    if (!res.ok) throw new Error(await res.text());
    return res.json();
}

export async function postAnalyzeConfig(config: AnalysisConfig): Promise<ConnectResponse> {
    const res = await fetch(`${BASE_URL}/analyzeConfig`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(config),
    });
    if (!res.ok) throw new Error(`POST /analyzeConfig failed: ${res.status}`);
    return res.json();
}

export async function fetchAnalyzeConfig(): Promise<AnalysisConfig> {
    const res = await fetch(`${BASE_URL}/analyzeConfig`);
    if (!res.ok) throw new Error(`GET /analyzeConfig failed: ${res.status}`);
    return res.json();
}