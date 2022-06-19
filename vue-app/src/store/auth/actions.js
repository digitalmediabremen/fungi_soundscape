// import Vue from 'vue'
import firebase from 'firebase'
import router from '@/router.js'
const actions = {
  async checkAuthentication ({ commit }) {
    await firebase
    firebase.auth().onAuthStateChanged((user) => {
      console.log('state', user)
      if (user) {
        // dispatch('autoSignIn', user)
        commit('set_user', user)
      } else {
        commit('clean_auth')
      }
    })
  },
  register: ({ commit }, data) => {
    return firebase
      .auth()
      .createUserWithEmailAndPassword(data.email, data.password).then((response) => {
      console.log("account created!", response)
      commit('set_user', response.user)
      var currentUser = firebase.auth().currentUser
      currentUser.updateProfile({
        displayName: data.name
      })
      return response.user
    }).catch((err) => {
      console.error(err)
      throw new Error(err)
    });

  },
  login: ({ commit }, data) => {
    return firebase
      .auth()
      .signInWithEmailAndPassword(data.email, data.password).then(response => {
        commit('set_user', response.user)
        return response.user
      })
      .catch(err => {
        console.error(err)
        throw new Error(err)
      });
  },
  logout: ({ commit }) => {
    return firebase
    .auth()
    .signOut()
    .then(() => {
      console.log('logout success')
      commit('clean_auth')
      router.push('/')
      return true
    })
  }
}

export default actions
