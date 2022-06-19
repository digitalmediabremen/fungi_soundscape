// import Vue from 'vue'

const getters = {
  getLastLocation: state => state.map.lastLocation,
  getUserPosition: state => state.map.userLocation
}

export default getters
