import { useEffect, useMemo, useState } from 'react';
import { toast } from 'react-hot-toast';
import { BASE_URL } from './api';
import { getDefaultConfig } from './helper';
import type { WulpusConfig } from './websocket-types';

type Props = {
    effectiveConfig: WulpusConfig;
    applyConfig?: (conf: WulpusConfig) => void;
};

async function listConfigs(): Promise<string[]> {
    const res = await fetch(`${BASE_URL}/configs`);
    if (!res.ok) throw new Error(await res.text());
    return res.json();
}

async function downloadConfig(filename: string): Promise<WulpusConfig> {
    const res = await fetch(`${BASE_URL}/configs/${encodeURIComponent(filename)}`);
    if (!res.ok) throw new Error(await res.text());
    return res.json();
}

async function deleteConfig(filename: string): Promise<void> {
    const res = await fetch(`${BASE_URL}/configs/${encodeURIComponent(filename)}`, {
        method: 'DELETE',
    });
    if (!res.ok) throw new Error(await res.text());
}

export function ConfigFilesPanel({ effectiveConfig, applyConfig }: Props) {
    const [saving, setSaving] = useState(false);
    const [saveName, setSaveName] = useState('');
    const [error, setError] = useState<string | null>(null);
    const [configList, setConfigList] = useState<string[] | null>(null);
    const [uploading, setUploading] = useState(false);

    const canSave = useMemo(() => {
        return !saving && effectiveConfig && typeof effectiveConfig === 'object';
    }, [saving, effectiveConfig]);

    const refreshList = async () => {
        try {
            setConfigList(await listConfigs());
        } catch (e) {
            setError(String(e));
        }
    };

    useEffect(() => { refreshList(); }, []);

    const onSave = async () => {
        setError(null);
        setSaving(true);
        try {
            const params = new URLSearchParams();
            if (saveName.trim()) params.set('name', saveName.trim());
            const res = await fetch(`${BASE_URL}/configs?${params.toString()}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(effectiveConfig),
            });
            if (!res.ok) throw new Error(await res.text());
            setSaveName('');
            await refreshList();
            toast.success('Configuration saved');
        } catch (e) {
            setError(String(e));
            toast.error('Failed to save configuration');
        } finally {
            setSaving(false);
        }
    };

    const handleApplyConfig = (config: WulpusConfig) => {
        applyConfig?.(config);
        toast.success('Configuration applied');
    };

    const applyFile = async (filename: string) => {
        setError(null);
        try {
            const conf = await downloadConfig(filename);
            handleApplyConfig(conf);
        } catch (e) {
            setError(String(e));
        }
    };

    const onDelete = async (filename: string) => {
        setError(null);
        const confirmed = window.confirm(`Delete config "${filename}"?`);
        if (!confirmed) return;
        try {
            await deleteConfig(filename);
            await refreshList();
            toast.success('Configuration deleted');
        } catch (e) {
            setError(String(e));
            toast.error('Failed to delete configuration');
        }
    };

    const applyLocalFile = async (event: React.ChangeEvent<HTMLInputElement>) => {
        const file = event.target.files?.[0];
        if (!file) return;

        setUploading(true);
        setError(null);

        try {
            const reader = new FileReader();
            reader.onload = async (e) => {
                const content = e.target?.result;
                if (typeof content === 'string') {
                    const config = JSON.parse(content);
                    handleApplyConfig(config);
                }
            };
            reader.readAsText(file);
        } catch (e) {
            setError(String(e));
        } finally {
            setUploading(false);
        }
    };

    return (
        <div className='p-4 space-y-2'>
            <h2 className="font-semibold">Config files</h2>
            {error && <div className="text-sm text-red-600">{error}</div>}
            <div className="flex flex-col sm:flex-row gap-2 items-start sm:items-end">
                <div className="flex-1">
                    <div className='flex flex-row gap-2 items-baseline'>
                        <label className="block text-sm font-medium text-gray-700">Filename</label>
                        <p className="text-xs text-gray-500 mt-1">(.json will be added automatically)</p>
                    </div>
                    <div className='flex flex-row gap-2 items-baseline flex-nowrap'>
                        <input
                            value={saveName}
                            onChange={(e) => setSaveName(e.target.value)}
                            placeholder="my-config"
                            className="mt-1 grow rounded-md border border-gray-300 px-3 py-2 focus:outline-none focus:ring-2 focus:ring-blue-600"
                        />
                        <button disabled={!canSave} onClick={onSave}
                            className="rounded-md bg-blue-600 hover:bg-blue-700  text-white text-sm px-4 py-2">
                            {saving ? 'Saving…' : 'Save current config'}
                        </button>
                    </div>
                </div>
            </div>

            <div>
                <div className="text-sm font-medium mb-2">Existing configs</div>
                {configList === null ? (
                    <div className="text-gray-500 text-sm">Loading…</div>
                ) : configList.length === 0 ? (
                    <div className="text-gray-500 text-sm">No config files found.</div>
                ) : (
                    <ul className="divide-y divide-gray-200">
                        {configList.map((f) => (
                            <li key={f} className="flex items-center justify-between py-2">
                                <span className="font-mono text-sm break-all">{f}</span>
                                <div className="flex items-center gap-2">
                                    <button onClick={() => applyFile(f)} className="text-blue-600 text-sm hover:underline">Load into UI</button>
                                    <a href={`/api/configs/${encodeURIComponent(f)}`} className="text-sm text-gray-700 hover:underline" download>Download</a>
                                    <button onClick={() => onDelete(f)} className="text-red-600 text-sm hover:underline">Delete</button>
                                </div>
                            </li>
                        ))}
                    </ul>
                )}
            </div>

            <div className='flex gap-3 justify-between text-sm'>
                <label className="rounded-md  bg-gray-100 hover:bg-gray-200 cursor-pointer px-2 py-1">
                    <input type="file" accept="application/json,.json" className="hidden" onChange={applyLocalFile} />
                    {uploading ? 'Uploading…' : 'Load local file (.json)'}
                </label>
                <button onClick={() => { handleApplyConfig(getDefaultConfig()); }} className="flex items-center gap-1 rounded-md bg-gray-100 hover:bg-gray-200 px-2 py-1">
                    <span className="material-symbols-rounded">settings_backup_restore</span>
                    Restore Default
                </button>
            </div>


        </div>
    );
}
