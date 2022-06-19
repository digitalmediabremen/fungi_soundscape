// import Vue from 'vue'
const actions = {
  setLastLocation: ({ commit }, location) => commit('storeLocation', location),
  fetchUserPosition: ({ commit, state }) => {
    console.warn('fetch position')
    return window.navigator.geolocation.getCurrentPosition((position) => {
       var center = {
        lat: position.coords.latitude,
        lng: position.coords.longitude,
        zoom: state.map.lastLocation.zoom
      }
      console.log('found pos!', center)
      commit('storeUserLocation', center)
      return center
    })
  }
}

export default actions
