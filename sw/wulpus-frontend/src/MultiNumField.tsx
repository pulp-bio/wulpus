import { useEffect, useRef, useState } from 'react';
import { CHANNEL_SIZE } from './App';

export function MultiNumField({ label, values, onChange, showChannelBoxes = false, color = 'bg-green-500' }: {
    label: string;
    values: number[];
    onChange: (vals: number[]) => void;
    showChannelBoxes?: boolean;
    color?: string;
}) {
    const [text, setText] = useState(values.join(','));
    const inputRef = useRef<HTMLInputElement | null>(null);

    // Only overwrite the text when the input is not focused. This prevents
    // removing a trailing comma while the user is typing (which made "," disappear).
    useEffect(() => {
        try {
            const active = typeof document !== 'undefined' ? document.activeElement : null;
            if (inputRef.current && active === inputRef.current) {
                // user is editing — don't clobber their input
                return;
            }
        } catch (e) {
            // ignore (e.g., SSR)
        }
        setText(values.join(','));
    }, [values]);

    function toggleChannel(ch: number) {
        const exists = values.includes(ch);
        const next = exists ? values.filter(v => v !== ch) : [...values, ch].sort((a, b) => a - b);
        onChange(next);
    }
    return (
        <label className="text-sm col-span-2">
            <div className="mb-1 text-gray-600">{label}</div>
            <div className="flex items-center gap-3 flex-wrap">
                <input ref={inputRef} className="flex-1 border rounded px-2 py-1"
                    value={text}
                    onChange={(e) => {
                        const v = e.target.value;
                        setText(v);
                        const nums = v.split(',').map(s => s.trim()).filter(Boolean).map(Number).filter(n => !Number.isNaN(n));
                        onChange(nums);
                    }}
                    placeholder="e.g., 0,1,2" />
                {showChannelBoxes && (
                    <div className="flex gap-1">
                        {Array.from({ length: CHANNEL_SIZE }).map((_, ch) => {
                            const active = values.includes(ch);
                            const base = `h-6 w-6 rounded flex items-center justify-center text-white text-xs ${active ? `${color} hover:saturate-80` : 'bg-gray-200 text-gray-700 hover:bg-gray-300'}`;
                            return (
                                <button key={ch} type="button" onClick={() => toggleChannel(ch)} className={base} aria-pressed={active} title={`Channel ${ch}`}>
                                    {ch}
                                </button>
                            );
                        })}
                    </div>
                )}
            </div>
        </label>
    );
}
