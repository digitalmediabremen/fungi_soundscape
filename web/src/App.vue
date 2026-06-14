<template>
  <div class="app">
    <header class="topbar">
      <div class="topbar__title">fungi <span>soundscape</span></div>
      <SourceToggle class="topbar__source" />
      <nav class="topbar__nav">
        <router-link to="/">Observe</router-link>
        <router-link to="/globe">Globe</router-link>
      </nav>
    </header>
    <main class="view">
      <router-view />
    </main>
  </div>
</template>

<script setup>
// App shell: top bar (title · data-source toggle · nav) + routed view.
import SourceToggle from './components/SourceToggle.vue'
</script>
