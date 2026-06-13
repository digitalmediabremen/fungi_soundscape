<template>
  <article class="card" v-if="fungus">
    <div class="card__image">
      <img
        v-if="fungus.imageUrl"
        :src="fungus.imageUrl"
        :alt="fungus.name"
        loading="lazy"
      />
      <div v-else class="card__noimage">no image</div>
    </div>

    <div class="card__body">
      <h2 class="card__name" :class="{ 'card__name--deprecated': info?.deprecated }">
        {{ fungus.name }}
      </h2>

      <p v-if="lineage" class="card__lineage">{{ lineage }}</p>

      <dl class="card__meta">
        <div v-if="fungus.locationName">
          <dt>where</dt>
          <dd>{{ fungus.locationName }}</dd>
        </div>
        <div v-if="fungus.date">
          <dt>when</dt>
          <dd>{{ fungus.date }}</dd>
        </div>
        <div v-if="fungus.confidence != null">
          <dt>confidence</dt>
          <dd>{{ fungus.confidence.toFixed(2) }}</dd>
        </div>
      </dl>

      <p v-if="description" class="card__desc">{{ description }}</p>

      <footer class="card__footer">
        <a :href="fungus.url" target="_blank" rel="noopener">view on mushroom observer ↗</a>
      </footer>
    </div>
  </article>
</template>

<script setup>
import { ref, computed, watch } from 'vue'
import { getNameInfo } from '../api/localData'

const props = defineProps({
  fungus: { type: Object, default: null },
})

// Species node (taxonomy + description), loaded lazily per name_id.
const info = ref(null)

const description = computed(
  () => info.value?.description || props.fungus?.description || ''
)

const lineage = computed(() => {
  const t = info.value?.taxonomy
  if (!t) return ''
  return [t.family, t.order, t.class].filter(Boolean).join(' · ')
})

watch(
  () => props.fungus?.nameId,
  async (nameId) => {
    info.value = null
    if (nameId == null) return
    try {
      info.value = await getNameInfo(nameId)
    } catch {
      info.value = null
    }
  },
  { immediate: true }
)
</script>

<style scoped>
.card {
  background: var(--bg-soft);
  border: 1px solid #1a201a;
  border-radius: 4px;
  overflow: hidden;
  box-shadow: var(--shadow);
  display: flex;
  flex-direction: column;
}

.card__image {
  background: #000;
  aspect-ratio: 4 / 3;
  display: flex;
  align-items: center;
  justify-content: center;
  overflow: hidden;
}
.card__image img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  display: block;
}
.card__noimage {
  color: var(--muted);
  letter-spacing: 1px;
}

.card__body {
  padding: 18px 20px 20px;
}

.card__name {
  margin: 0 0 4px;
  font-size: 22px;
  font-style: italic;
  color: var(--accent);
  word-break: break-word;
}
.card__name--deprecated {
  text-decoration: line-through;
  text-decoration-thickness: 1px;
  opacity: 0.85;
}

.card__lineage {
  margin: 0 0 14px;
  font-size: 12px;
  letter-spacing: 1px;
  text-transform: uppercase;
  color: var(--muted);
}

.card__meta {
  margin: 0 0 14px;
  display: grid;
  gap: 6px;
}
.card__meta > div {
  display: grid;
  grid-template-columns: 92px 1fr;
  gap: 10px;
  font-size: 13px;
}
.card__meta dt {
  color: var(--muted);
  text-transform: uppercase;
  letter-spacing: 1px;
}
.card__meta dd {
  margin: 0;
}

.card__desc {
  font-size: 14px;
  line-height: 1.55;
  color: #cdd8cd;
  max-height: 9.5em;
  overflow: auto;
  margin: 0 0 16px;
}

.card__footer {
  display: flex;
  flex-direction: column;
  gap: 6px;
  font-size: 12px;
  border-top: 1px solid #1a201a;
  padding-top: 12px;
}
</style>
