// Simple API client for the FastAPI backend

export type ConnectResponse = { ok: string } | { [key: string]: string };

export type ConnectionType = 'serial' | 'ble';
export type ConnectionOption = {
    device: string;           // e.g. COM5 or BLE MAC/address
    description: string;      // human-friendly label
    type: ConnectionType;     // 'serial' | 'ble'
};

// Use Vite proxy in dev to avoid CORS; see vite.config.ts
export const BASE_URL = "/api";

export async function getBTHConnections(): Promise<ConnectionOption[]> {
    const res = await fetch(`${BASE_URL}/connections`);
    if (!res.ok) throw new Error(`GET /connections failed: ${res.status}`);
    const data = await res.json();
    if (Array.isArray(data)) {
        // Expecting [{ device, description, type }, ...]
        return (data as unknown[])
            .filter(Boolean)
            .map((item) => {
                const obj = item as Partial<ConnectionOption> & Record<string, unknown>;
                const t = obj.type === 'ble' || obj.type === 'serial' ? obj.type : 'serial';
                return {
                    device: String(obj.device ?? ''),
                    description: String(obj.description ?? obj.device ?? ''),
                    type: t,
                } as ConnectionOption;
            });
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

export async function postDisconnect(): Promise<void> {
    const res = await fetch(`${BASE_URL}/disconnect`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
    });
    if (!res.ok) throw new Error(`POST /disconnect failed: ${res.status}`);
}

export async function postStart(config: unknown): Promise<ConnectResponse> {
    const res = await fetch(`${BASE_URL}/start`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(config),
    });
    if (!res.ok) throw new Error(`POST /start failed: ${res.status}`);
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