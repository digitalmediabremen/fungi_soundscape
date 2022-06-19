<template>
  <v-col
    v-if="story"
    class="story fill-height fill-width"
    align="center"
    justify="center"
    align-content="center"
  >
    <v-app-bar class="story_header" color="black accent-4" dense dark>
      <v-toolbar-title>{{ story.title }}</v-toolbar-title>
    </v-app-bar>
    <v-col class="story_body">
      <MapBox class="map" :stories="[story]" locked />
      <p class="story_body_description">{{ story.description }}</p>
      <v-carousel cycle height="400" hide-delimiter-background show-arrows-on-hover>
        <v-carousel-item v-for="(image, i) in story.images" :src="image.downloadUrl" :key="i"></v-carousel-item>
      </v-carousel>
    </v-col>
  </v-col>
</template>
<script>
import { mapActions, mapGetters } from 'vuex'
import {
  MapBox
} from '@/components'
export default {
  name: 'newstorypick',
  components: {
    MapBox
  },
  computed: {
        ...mapGetters('story',
      [
        'getStories'
      ])
  },
  mounted () {
    this.setCurrentStory(null)
    this.fetchStories().then(() => {
      this.story = this.getStories.find((story) => {
        return story.id === this.$route.params.id
      })
    })
  },
  data: () => ({
    story: null
  }),

  methods: {
    ...mapActions('auth', [
      'login',
      'getUser'
    ]),
    ...mapActions('story', [
      'fetchStories',
      'getUser'
    ]),
    ...mapActions('explore', [
      'setCurrentStory'
    ]),
    submit () {
    },
    reset () {
      this.$refs.form.reset()
    },
    resetValidation () {
      this.$refs.form.resetValidation()
    }
  }
}
</script>

<style lang='sass' scoped>
.map
  pointer-events: none !important
  margin-top: 5px
  width: 100vw !important
  height: 150px !important

.story
  height: 100%
  &_header
    position: fixed
    z-index: 1000
  &_body
    height: calc(100vh - 140px)
    margin-top: 50px
    overflow-x: hidden
    overflow-y: scroll
    &_description
      margin-top: 20px
      margin-bottom: 20px
      height: fit-content
      text-align: left
      color: black
</style>