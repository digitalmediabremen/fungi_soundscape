import Vue from 'vue'
import App from './App.vue'
import router from './router'
import store from './store/index'
import './registerServiceWorker'
import firebase from 'firebase'
import vuetify from './plugins/vuetify';
import Clipboard from 'v-clipboard'

Vue.config.productionTip = false

new Vue({
  router,
  store,
  vuetify,
  render: h => h(App)
}).$mount('#app')

Vue.use(Clipboard)


// Your web app's Firebase configuration
var firebaseConfig = {
  apiKey: process.env.VUE_APP_FIREBASE_APIKEY,
  authDomain: process.env.VUE_APP_FIREBASE_AUTHDOMAIN,
  databaseURL: process.env.VUE_APP_FIREBASE_DATABASEURL,
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

if (firebase.messaging.isSupported()) {
  console.warn('messaging supported!')
  const messaging = firebase.messaging()
  messaging.usePublicVapidKey(process.env.VUE_APP_FIREBASE_MESSAGING) // 1. Generate a new key pair
  // Request Permission of Notifications
  messaging.requestPermission().then(() => {
    console.log('Notification permission granted.')

    // Get Token
    messaging.getToken().then((token) => {
      console.log(token)
    })
  }).catch((err) => {
    console.log('Unable to get permission to notify.', err)
  });
} else {
  console.error('messaging not supported!')
}
