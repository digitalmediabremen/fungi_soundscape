// import Vue from 'vue'

const getters = {
  getStories: state => state.story.list,
  getCreatingStory: state => state.story.creating
}

export default getters
