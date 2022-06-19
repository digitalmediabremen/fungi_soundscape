// import Vue from 'vue'
import api from '@/api'
import { db } from "@/main.js" // <--- or wherever the config file is

const actions = {
  async fetchStories ({ commit }) {
    console.log('fetch stories')
    await db
    return db.collection("stories").get().then((snapshot) => {
      console.warn('stories!', snapshot)
      commit('storeStories', snapshot.docs.map(doc => {
        return {
          id: doc.id,
          ...doc.data()
        }
      }))
    })
  },
  async save ({ dispatch, commit }, story) {
    console.log('save stories')
    let save = story
    console.log('saving', save)
    await db
    commit('clear_creating_story')
    return db.collection("stories").add(save).then(() => dispatch('fetchStories'))
  },
  fetchStoryTracks: ({ commit }, id) => {
    return api.soundcloud
      .fetch().then((response) => {
        console.log(response)
        commit('storeTracks', { id: id, tracks: response.collection })
        return response.collection
      }).catch((e) => {
        return e
      })
  },
  storeCreatingStory: ({ commit, state }, story) => {
    let merged = {
      ...state.story.creating,
      ...story,
    }
    console.log('merged', merged)
    commit('store_creating_story', merged)
  }
}

export default actions
