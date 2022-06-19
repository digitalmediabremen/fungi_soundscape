const mutations = {
  storeStories: (state, value) => {
    console.warn('stored', value)
    state.story.list = value
  },
  storeTracks: (state, value) => {
    console.log('story tracks:', value)
    state.story.list.find((story) => story.id === parseInt(value.id)).tracks = value.tracks
    console.log(state.story.list)
  },
  store_creating_story: (state, value) => {
    state.story.creating = value
  },
  clear_creating_story: (state) => {
    state.story.creating = null
  }
}

export default mutations
