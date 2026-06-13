<template>
  <div class="app">
    <header class="topbar">
      <div class="topbar__title">fungi <span>soundscape</span></div>
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
// App shell: top bar + routed view. Nothing stateful here yet.
</script>
