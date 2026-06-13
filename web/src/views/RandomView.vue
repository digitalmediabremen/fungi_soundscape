<template>
  <div class="random">
    <div class="random__stage">
      <transition name="fade" mode="out-in">
        <!-- Loading -->
        <div v-if="loading" key="loading" class="random__center">
          <div class="spinner" />
          <p class="random__hint">summoning a fungus…</p>
        </div>

        <!-- Error -->
        <div v-else-if="error" key="error" class="random__center">
          <p class="error">{{ error }}</p>
          <button class="btn" @click="observe">try again</button>
        </div>

        <!-- Result -->
        <div v-else-if="fungus" key="result" class="random__center">
          <MushroomCard :fungus="fungus" class="random__card" />
          <button class="btn" :disabled="loading" @click="observe">observe another</button>
        </div>

        <!-- Idle -->
        <div v-else key="idle" class="random__center">
          <p class="random__intro">
            A random fungus, observed somewhere on Earth.
          </p>
          <button class="btn btn--big" @click="observe">observe</button>
        </div>
      </transition>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import MushroomCard from '../components/MushroomCard.vue'
import { fetchRandomObservation } from '../api/localData'

const loading = ref(false)
const error = ref('')
const fungus = ref(null)

async function observe() {
  loading.value = true
  error.value = ''
  try {
    fungus.value = await fetchRandomObservation()
  } catch (e) {
    error.value = e.message || 'Something went wrong.'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.random {
  height: 100%;
  overflow-y: auto;
}
.random__stage {
  min-height: 100%;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 32px 16px;
}
.random__center {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 22px;
}
.random__card {
  width: min(440px, 92vw);
}
.random__intro {
  color: var(--muted);
  font-size: 16px;
  text-align: center;
  max-width: 360px;
}
.random__hint {
  color: var(--muted);
  letter-spacing: 1px;
}

.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.2s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
