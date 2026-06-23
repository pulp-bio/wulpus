import { useEffect, useState } from 'react';
import { useNavigate } from "react-router";
import { BASE_URL, replayFile } from './api';

export function LogsPage() {
    const [files, setFiles] = useState<string[] | null>(null);
    const [error, setError] = useState<string | null>(null);
    const navigate = useNavigate();

    useEffect(() => {
        let cancelled = false;
        fetch(`${BASE_URL}/logs`)
            .then(async (res) => {
                if (!res.ok) throw new Error(await res.text());
                return res.json();
            })
            .then((data: string[]) => { if (!cancelled) setFiles(data); })
            .catch((e) => { if (!cancelled) setError(String(e)); });
        return () => { cancelled = true; };
    }, []);

    const handleReplay = (filename: string) => {
        // Implement replay functionality here
        replayFile(filename)
            .then(() => {
                navigate("/");
            })
            .catch((e) => {
                setError(String(e));
            });
    };

    return (
        <div className="min-h-screen bg-gray-50 text-gray-900">
            <main className="mx-auto max-w-5xl px-4 sm:px-6 lg:px-8 py-6 space-y-4">
                <header className="flex items-center justify-between">
                    <h1 className="text-2xl font-semibold">Recorded logs</h1>
                    <a href="/" className="text-blue-600 hover:underline">Back to Dashboard</a>
                </header>

                <div className="bg-white rounded-lg shadow">
                    <div className="p-4">
                        {error && (
                            <div className="text-red-600">{error}</div>
                        )}
                        {files === null ? (
                            <div className="text-gray-500">Loading…</div>
                        ) : files.length === 0 ? (
                            <div className="text-gray-500">No logs found.</div>
                        ) : (
                            <ul className="divide-y divide-gray-200">
                                {files.map((f) => (
                                    <li key={f} className="flex items-center gap-3 py-2">
                                        <span className="font-mono text-sm break-all grow">{f}</span>
                                        <button
                                            className="inline-flex items-center gap-2 rounded-md bg-blue-600 text-white text-sm px-3 py-1.5 hover:bg-blue-700"
                                            onClick={() => handleReplay(f)}
                                        >
                                            Replay
                                        </button>
                                        <a
                                            className="inline-flex items-center gap-2 rounded-md bg-gray-600 text-white text-sm px-3 py-1.5 hover:bg-gray-700"
                                            href={`/logs/${encodeURIComponent(f)}`}
                                            download
                                        >
                                            Download
                                        </a>
                                    </li>
                                ))}
                            </ul>
                        )}
                    </div>
                </div>
            </main>
        </div>
    );
}
