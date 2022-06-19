// import Vue from 'vue'
const actions = {
  startAnimation: ({ commit, getters, rootGetters, state }) => {
    if (getters.getIsAnimating) {
      console.warn("already animating")
      return
    }
    if (rootGetters['story/getStories'].length === 0) {
      console.warn("no stories to animate")
      return
    }
    commit('set_index', 0)
    commit('set_current_story', rootGetters['story/getStories'][0])

    console.log('first', rootGetters['story/getStories'][0])

    let animation = setInterval(() => {
      let index = state.explore.index
      console.log('interval to animate other')
      index++
      if (index > rootGetters['story/getStories'].length - 1) {
        index = 0
      }
      commit('set_current_story', rootGetters['story/getStories'][index])
      commit('set_index', index)
    }, 6000)
    commit('set_animation', animation)

  },
  setCurrentStory: ({ commit }, story) => {
    commit('set_current_story', story)
  },
  stopAnimation: ({ commit, getters }) => {
    clearInterval(getters.getAnimation)
    commit('set_no_animation')
  },
}

export default actions
