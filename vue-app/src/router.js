import Vue from 'vue'
import Router from 'vue-router'
import Home from './views/Home.vue'
import Explore from './views/Explore.vue'
import Login from './views/Login.vue'
import Register from './views/Register.vue'
import NewStory from './views/NewStory.vue'
import NewStoryPick from './views/NewStoryPick.vue'
import NewStoryFiles from './views/NewStoryFiles.vue'
import Story from './views/Story.vue'

import store from './store/index'

Vue.use(Router)

const router = new Router({
  mode: 'history',
  routes: [
    {
      path: '/',
      name: 'home',
      component: Home,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/explore',
      name: 'explore',
      component: Explore,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/login',
      name: 'login',
      component: Login,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/register',
      name: 'register',
      component: Register,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/create/1',
      name: 'create',
      component: NewStory,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/create/2',
      name: 'create2',
      component: NewStoryFiles,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/create/3',
      name: 'create3',
      component: NewStoryPick,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    },
    {
      path: '/story/:id',
      name: 'story',
      component: Story,
      meta: {
        auth: false,
        top: {
          show: true
        }
      }
    }
  ]
})

router.beforeEach((to, from, next) => {
  const requiresAuth = to.matched.some(record => record.meta.auth)
  const isAuthenticated = store.getters['auth/isAuthenticated']
  if (requiresAuth && !isAuthenticated) {
    next('/')
  } else {
    next()
  }
})


export default router