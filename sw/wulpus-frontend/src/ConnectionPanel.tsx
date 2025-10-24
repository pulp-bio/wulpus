import { useCallback, useEffect, useState } from "react";
import { toast } from 'react-hot-toast';
import { deactivateMock, getBTHConnections, postActivateMock, postConnect, postDisconnect, postStart, postStop, type ConnectionOption } from "./api";
import { StatusView } from "./StatusView";
import type { Status, WulpusConfig } from "./websocket-types";

export function ConnectionPanel(props: { effectiveConfig: WulpusConfig, status: Status | null, allStatuses?: Status[] }) {
    const { effectiveConfig, status, allStatuses } = props;
    const [connections, setConnections] = useState<ConnectionOption[]>([]);
    const [selectedConnection, setSelectedConnection] = useState<string>("");
    const [isRefreshing, setIsRefreshing] = useState<boolean>(false);

    const isMock = allStatuses?.some(status => status.mock) ?? false;
    const someJobRunning = allStatuses?.some(s => s.status === 3) ?? false;

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
    }, [selectedConnection]);


    useEffect(() => {
        refreshConnections()
    }, [refreshConnections]);

    async function handleConnect(connection: string) {
        console.log("Connecting to port:", connection);
        if (!connection) return;
        await postConnect(connection);
    }

    async function handleDisconnect(connection: string) {
        console.log("Disconnecting from port:", connection);
        if (!connection) return;
        await postDisconnect(connection);
    }

    async function handleStart() {
        toast('Starting...');
        try {
            await postStart(effectiveConfig);
            toast.success('Started');
        } catch (e) {
            if (e instanceof Error) {
                toast.error(`Failed to start: ${e.message}`);
            }
        }
    }


    return (
        <div className="p-4 space-y-3">
            <div className="flex gap-2 flex-row items-center">
                <h2 className="font-medium ">Connections {isMock ? ' (Simulation)' : ''}</h2>
                <div className="grow" />
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
                <button onClick={refreshConnections} title="Refresh" className="p-1 bg-gray-100 hover:bg-gray-200 flex items-center rounded">
                    <span className={`material-symbols-rounded ${isRefreshing ? 'motion-safe:animate-spin' : ''}`}>refresh</span>
                </button>
            </div>
            <div className="flex flex-col space-y-2">
                {allStatuses?.sort((a, b) => b.status - a.status).map((s, idx) => (
                    <StatusView key={idx} status={s} handleConnect={handleConnect} handleDisconnect={handleDisconnect} connections={connections} />
                ))}
                {!someJobRunning && allStatuses?.every(s => s.status !== 0) && (
                    <StatusView handleConnect={handleConnect} handleDisconnect={handleDisconnect} connections={connections} disabled={someJobRunning} />
                )}
                <div className='flex space-x-2'>
                    {allStatuses?.[0]?.status === undefined && (
                        <div className="font-medium text-red-500 border-1 px-2 border-red-500 hover:bg-gray-50 rounded">Server not running!</div>
                    )}

                    {allStatuses?.[0]?.status !== undefined && !someJobRunning && (
                        <button
                            onClick={handleStart}
                            className={`w-full bg-green-600 hover:bg-green-700 text-white rounded px-3 py-2 disabled:opacity-50`}
                            disabled={someJobRunning}
                        >
                            Start
                        </button>
                    )}
                    {someJobRunning && (
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
                Progress: {Math.round((status?.progress ?? 0) * 100)}%
            </div>
        </div>
    )
}