import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store/index'
import './registerServiceWorker'
import vuetify from './plugins/vuetify';
import Clipboard from 'v-clipboard'
import VuePapaParse from 'vue-papa-parse'
Vue.config.productionTip = false
import { getDatabase } from "firebase/database"
import { initializeApp } from "firebase/app"

import Vue2TouchEvents from 'vue2-touch-events'

Vue.use(Vue2TouchEvents)
Vue.use(Clipboard)
Vue.use(VuePapaParse)

new Vue({
  router,
  store,
  vuetify,
  render: h => h(App)
}).$mount('#app')


// Your web app's Firebase configuration
var firebaseConfig = {
  apiKey: process.env.VUE_APP_FIREBASE_APIKEY,
  authDomain: process.env.VUE_APP_FIREBASE_AUTHDOMAIN,
  projectId: process.env.VUE_APP_FIREBASE_PROJECTID,
  storageBucket: process.env.VUE_APP_FIREBASE_STORAGEBUCKET,
  messagingSenderId: process.env.VUE_APP_FIREBASE_SENDERID,
  appId: process.env.VUE_APP_FIREBASE_APPID,
  databaseURL: process.env.VUE_APP_DATABASE_URL,
};
// Initialize Firebase
const app = initializeApp(firebaseConfig)

// eslint-disable-next-line
export const db = getDatabase(app)

