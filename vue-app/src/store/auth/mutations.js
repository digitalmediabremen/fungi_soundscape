const mutations = {
  set_user: (state, user) => {
    state.auth.user = user
    state.auth.authenticated = true
  },
  clean_auth: (state) => {
    state.auth.user = null
    state.auth.authenticated = false
  }
}

export default mutations
