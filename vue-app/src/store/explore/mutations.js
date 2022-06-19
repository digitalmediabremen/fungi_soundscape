const mutations = {
  set_animation: (state, animation) => {
    state.explore.animation = animation
  },
  set_no_animation: (state) => {
    state.explore.animation = null
    state.explore.index = 0
    state.explore.currentStory = null
  },
  set_current_story: (state, value) => {
    state.explore.currentStory = value
  },
  set_index: (state, value) => {
    state.explore.index = value
  }
}

export default mutations
