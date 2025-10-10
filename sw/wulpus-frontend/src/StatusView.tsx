import { useState } from "react";
import { StatusLabel, type ConnectionOption } from "./api";
import type { Status } from "./websocket-types";

type Handler = (connection: string) => void

export function StatusView(props: {
    status?: Status,
    handleConnect?: Handler,
    handleDisconnect?: Handler,
    connections: ConnectionOption[]
}) {
    const { status, handleConnect, handleDisconnect, connections } = props;
    const [selectedConnection, setSelectedConnection] = useState<string>("");

    const commonButtonStyle: React.HTMLAttributes<HTMLButtonElement>['className'] = "text-white rounded items-center inline-flex justify-center px-3 py-2 h-9 max-w-23";

    const connectionValue = ((status?.status ?? 0) !== 0) ? (status?.endpoint ?? "") : selectedConnection;
    return (
        <div className="flex flex-col">
            <div className="flex flex-row flex-nowrap items-center space-x-2">
                <select className="border rounded px-2 py-1 w-52 grow h-9 disabled:bg-gray-200"
                    disabled={(status?.status ?? 0) !== 0}
                    value={connectionValue}
                    onChange={(e) => setSelectedConnection(e.target.value)}>
                    <option value="">Select port</option>
                    {connections.map((c) => (
                        <option key={`${c.type}:${c.device}`} value={c.device}>
                            {c.type === 'ble' ? 'BLE: ' : ''}{c.description}
                        </option>
                    ))}
                </select>
                {(status === undefined || status?.status === 0) && (
                    <button
                        onClick={() => handleConnect?.(selectedConnection)}
                        className={`w-full bg-blue-600 hover:bg-blue-700 ${commonButtonStyle}`}
                    >
                        Connect
                    </button>
                )}
                {status?.status !== undefined && status.status !== 0 && (
                    <button
                        onClick={() => handleDisconnect?.(connectionValue)}
                        className={`w-full bg-yellow-600 hover:bg-yellow-700 ${commonButtonStyle} ${status?.status === 1 ? 'opacity-50' : ''}`}
                    >
                        Disconnect
                    </button>
                )}
            </div>
            {!!status &&
                <div className="text-xs text-gray-600">
                    Status: {status ? StatusLabel(status.status) : 'No Server/Backend'} · BT {status?.bluetooth ?? '—'}
                </div>
            }
        </div>
    )
}