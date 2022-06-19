const mutations = {
  storeLocation: (state, value) => {
    state.map.lastLocation = value
  },
  storeUserLocation: (state, value) => {
    state.map.userLocation = value
    state.map.lastLocation = value
  }
}

export default mutations
