// import Vue from 'vue'

const getters = {
  getAnimation: state => state.explore.animation,
  getIsAnimating: state => {
    if (state.explore.animation !== null) {
      return true
    }
    return false
  },
  getCurrentStory: state => state.explore.currentStory
}

export default getters
