import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store/index'
import './registerServiceWorker'
import firebase from 'firebase'
import vuetify from './plugins/vuetify';
import Clipboard from 'v-clipboard'
import VuePapaParse from 'vue-papa-parse'
Vue.config.productionTip = false


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
  appId: process.env.VUE_APP_FIREBASE_APPID
};
// Initialize Firebase
firebase.initializeApp(firebaseConfig)

// database
export const db = firebase.firestore()

// storage
export const storage = firebase.storage().ref();

