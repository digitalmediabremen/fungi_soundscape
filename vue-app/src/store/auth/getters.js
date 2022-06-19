// import Vue from 'vue'

const getters = {
  getUser: state => state.auth.user,
  isAuthenticated: state  => state.auth.authenticated
}

export default getters
