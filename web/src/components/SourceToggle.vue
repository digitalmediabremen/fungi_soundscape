<template>
  <div class="src" role="group" aria-label="data source">
    <span class="src__label">data</span>
    <button
      v-for="s in sources"
      :key="s.id"
      class="src__btn"
      :class="{ 'src__btn--on': s.id === current }"
      :disabled="available[s.id] === false"
      :title="available[s.id] === false ? `not built — run npm run build:${s.id === 'gbif' ? 'gbif' : 'data'}` : s.label"
      @click="choose(s.id)"
    >
      {{ s.short }}
    </button>
  </div>
</template>

<script setup>
import { reactive } from 'vue'
import { sources as sourceMap, source, setSource } from '../api/source'

const sources = Object.values(sourceMap)
const current = source.id

// null = unknown (probing), true/false = built or not.
const available = reactive(Object.fromEntries(sources.map((s) => [s.id, null])))

// The active source is obviously available (we're reading it); probe the others.
available[current] = true
const base = import.meta.env.BASE_URL
for (const s of sources) {
  if (s.id === current) continue
  fetch(`${base}${s.dir}meta.json`, { method: 'HEAD' })
    .then((res) => {
      available[s.id] = res.ok
    })
    .catch(() => {
      available[s.id] = false
    })
}

function choose(id) {
  if (available[id] === false) return
  setSource(id)
}
</script>

<style scoped>
.src {
  display: flex;
  align-items: center;
  gap: 4px;
}
.src__label {
  color: var(--muted);
  font-size: 11px;
  letter-spacing: 1px;
  text-transform: uppercase;
  margin-right: 4px;
}
.src__btn {
  background: transparent;
  color: var(--muted);
  border: 1px solid #1a201a;
  border-radius: 2px;
  padding: 5px 10px;
  font-size: 11px;
  letter-spacing: 1px;
  text-transform: uppercase;
  transition: color 0.15s, border-color 0.15s;
}
.src__btn:hover:not(:disabled):not(.src__btn--on) {
  color: var(--fg);
}
.src__btn--on {
  color: var(--accent);
  border-color: var(--accent-dim);
}
.src__btn:disabled {
  opacity: 0.35;
  cursor: not-allowed;
}
</style>
