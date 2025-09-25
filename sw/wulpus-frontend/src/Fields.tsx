// UI helpers
export function NumberField({ label, value, onChange, step = 1, min }: { label: string; value: number; onChange: (v: number) => void; step?: number; min?: number; }) {
  return (
    <label className="text-sm">
      <div className="mb-1 text-gray-600">{label}</div>
      <input type="number" className="w-full border rounded px-2 py-1" value={value} step={step} min={min}
        onChange={(e) => onChange(Number(e.target.value))} />
    </label>
  );
}
export function SelectField({ label, value, onChange, options }: { label: string; value: string | number; onChange: (v: string) => void; options: { value: string; label: string; }[]; }) {
  return (
    <label className="text-sm">
      <div className="mb-1 text-gray-600">{label}</div>
      <select className="w-full border rounded px-2 py-1" value={String(value)} onChange={(e) => onChange(e.target.value)}>
        {options.map(o => <option key={o.value} value={o.value}>{o.label}</option>)}
      </select>
    </label>
  );
}

export function StringField({ label, value, onChange, placeholder }: { label: string; value: string; onChange: (v: string) => void; placeholder?: string; }) {
  return (
    <label className="text-sm">
      <div className="mb-1 text-gray-600">{label}</div>
      <input
        type="text"
        className="w-full border rounded px-2 py-1"
        value={value}
        placeholder={placeholder}
        onChange={(e) => onChange(e.target.value)}
      />
    </label>
  );
}
