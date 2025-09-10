import { useEffect, useState, useCallback } from "react";
import { deactivateMock, getBTHConnections, postActivateMock, postConnect, postDisconnect, postStart, postStop, StatusLabel, type ConnectionOption } from "./api";
import type { Status, WulpusConfig } from "./websocket-types";
import { toast } from 'react-hot-toast';

export function ConnectionPanel(props: { effectiveConfig: WulpusConfig, status: Status | null }) {
    const { effectiveConfig, status } = props;
    const [connections, setConnections] = useState<ConnectionOption[]>([]);
    const [selectedConnection, setSelectedConnection] = useState<string>("");
    const [isRefreshing, setIsRefreshing] = useState<boolean>(false);

    const isMock = status?.mock ?? false;

    const refreshConnections = useCallback(async () => {
        setIsRefreshing(true);
        toast('scanning...', { icon: <span className="material-symbols-rounded">bluetooth</span> });
        const list = await getBTHConnections();
        setIsRefreshing(false);
        setConnections(list);
        const justPosts: string[] = list.map(item => item.device);
        if (selectedConnection && !justPosts.includes(selectedConnection)) {
            setSelectedConnection("");
        }
    }, []);


    useEffect(() => {
        refreshConnections()
    }, [refreshConnections]);

    async function handleConnect() {
        console.log("Connecting to port:", selectedConnection);
        if (!selectedConnection) return;
        await postConnect(selectedConnection);
    }

    async function handleStart() {
        toast('Starting...');
        await postStart(effectiveConfig);
    }

    return (
        <div className="p-4 space-y-3">
            <div className="flex gap-2 flex-row items-center">
                <h2 className="font-medium grow">Connection {isMock ? ' (Simulation)' : ''}</h2>
                {isMock &&
                    <>
                        <button
                            onClick={deactivateMock}
                            className="font-medium text-red-500 border-1 px-2 border-red-500 hover:bg-gray-50 rounded"
                        >
                            Stop Simulation
                        </button>
                    </>
                }
                {!isMock && (
                    <button onClick={postActivateMock} title="Activate Simulation" className="hover:bg-gray-100 text-gray-800 flex items-center rounded">
                        <span className="material-symbols-rounded">smart_toy</span>
                    </button>
                )}
            </div>
            <div className="flex flex-col space-y-2">
                <div className="flex flex-row flex-nowrap items-center space-x-2">
                    <select className="border rounded px-2 py-1 w-52"
                        disabled={(status?.status ?? 0) !== 0}
                        value={selectedConnection}
                        onChange={(e) => setSelectedConnection(e.target.value)}>
                        <option value="">Select port</option>
                        {connections.map((c) => (
                            <option key={`${c.type}:${c.device}`} value={c.device}>
                                {c.type === 'ble' ? 'BLE: ' : ''}{c.description}
                            </option>
                        ))}
                    </select>
                    <button onClick={refreshConnections} title="Refresh" className="p-1 bg-gray-100 hover:bg-gray-200 flex items-center rounded">
                        <span className={`material-symbols-rounded ${isRefreshing ? 'motion-safe:animate-spin' : ''}`}>refresh</span>
                    </button>
                </div>
                <div className='flex space-x-2'>
                    {status?.status === undefined && (
                        <div className="font-medium text-red-500 border-1 px-2 border-red-500 hover:bg-gray-50 rounded">Server not running!</div>
                    )}
                    {status?.status === 0 && (
                        <button
                            onClick={handleConnect}
                            className={`w-full bg-blue-600 hover:bg-blue-700 text-white rounded px-3 py-2 `}
                        >
                            Connect
                        </button>
                    )}
                    {status?.status !== undefined && status.status !== 0 && (
                        <button
                            onClick={postDisconnect}
                            className={`w-full bg-yellow-600 hover:bg-yellow-700 text-white rounded px-3 py-2 ${status?.status === 1 ? 'opacity-50' : ''}`}
                        >
                            Disconnect
                        </button>
                    )}

                    {status?.status !== undefined && status.status !== 3 && (
                        <button
                            onClick={handleStart}
                            className={`w-full bg-green-600 hover:bg-green-700 text-white rounded px-3 py-2 ${status?.status != 2 ? 'opacity-50' : ''}`}
                            disabled={status?.status != 2}
                        >
                            Start
                        </button>
                    )}
                    {status?.status === 3 && (
                        <button
                            onClick={postStop}
                            className={`w-full bg-red-600 hover:bg-red-700 text-white rounded px-3 py-2`}
                        >
                            Stop
                        </button>
                    )}
                </div>
            </div>
            <div className="text-xs text-gray-600">
                Status: {status ? StatusLabel(status.status) : 'No Server/Backend'} · BT: {status?.bluetooth ?? '—'} · Progress: {Math.round((status?.progress ?? 0) * 100)}%
            </div>
        </div>
    )
}