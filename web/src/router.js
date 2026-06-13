import { createRouter, createWebHashHistory } from 'vue-router'
import RandomView from './views/RandomView.vue'

// Hash history keeps deployment trivial on static hosts / subpaths
// (no server-side SPA fallback needed).
// GlobeView is lazy-loaded so three.js/globe.gl only download when needed,
// keeping the initial "Observe" page light.
const routes = [
  { path: '/', name: 'random', component: RandomView },
  { path: '/globe', name: 'globe', component: () => import('./views/GlobeView.vue') },
]

export default createRouter({
  history: createWebHashHistory(),
  routes,
})
