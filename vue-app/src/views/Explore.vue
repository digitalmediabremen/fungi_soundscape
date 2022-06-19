<template>
  <v-col class="explore">
    <v-app-bar
      color="black accent-4"
      dense
      dark
    >
      <v-autocomplete
        align-self-center
        v-model="searchedStory"
        :items="getStories"
        height="30"
        color="white"
        hide-no-data
        hide-selected
        item-text="title"
        item-value="id"
        hide-details
        placeholder="Search"
        prepend-icon="mdi-database-search"
        return-object
      ></v-autocomplete>
      <!--<v-toolbar-title>Explore</v-toolbar-title>-->
      <v-btn
        tile
        dark
        color="white"
        outlined
        @click="addNew"
        class="ml-4"
      >
      Report incident
      </v-btn>
    </v-app-bar>
    <div class="explore_content">
      <div class="controls">
        <div class="controls_arrow_left" @click="nextStory">
          <img
            src="@/assets/icons/arrow.png"
          >
        </div>
        <div class="controls_arrow_right" @click="previousStory">
          <img
            src="@/assets/icons/arrow.png"
          >
        </div>
      </div>
      <MapBox
        :stories="getStories"
        class="map"
        @loaded="onLoaded"
      />
    </div>
  </v-col>
</template>

<script>
import { mapGetters, mapActions } from 'vuex'

import {
  MapBox
} from '@/components'

export default {
  components: {
    MapBox
  },
  computed: {
    ...mapGetters('story',
      [
        'getStories'
      ]),
      ...mapGetters('explore',
      [
        'getCurrentStory',
        'getIsAnimating'
      ]),
      ...mapGetters('auth', [
        'isAuthenticated'
      ])
  },
  data () {
    return {
      searchedStory: null,
      lastIndex: 0,
      nextTimeout: false
    }
  },
  mounted () {
    this.fetchUserPosition()
    this.fetchStories()
  },
  destroyed () {
    this.stopAnimation()
  },
  methods: {
    async onLoaded () {
      await this.getStories.length > 0
      console.log('loaded, go start anim')
      // this.startAnimation()
    },
    addNew () {
      if (this.isAuthenticated) {
        this.$router.push('/create/1')
      } else {
        this.$router.push(
          {
            path: '/login',
            query: {
              creatingEntry: true
          }
        })
      }
    },
    previousStory () {
      if (this.nextTimeout) {
        return
      }

      this.nextTimeout = true

      setTimeout(() => {
        this.nextTimeout = false
      }, 300)
      if (this.getIsAnimating) {
        this.stopAnimation()
      }

      if (this.getCurrentStory === null) {
        this.setCurrentStory(this.getStories[this.lastIndex])
      } else {
        this.lastIndex--
        if (this.lastIndex < 0) {
          this.lastIndex = this.getStories.length - 1
        }
        this.setCurrentStory(this.getStories[this.lastIndex])
        this.searchedStory = this.getStories[this.lastIndex]
      }
    },
    nextStory () {
      if (this.nextTimeout) {
        return
      }

      this.nextTimeout = true

      setTimeout(() => {
        this.nextTimeout = false
      }, 300)
      if (this.getIsAnimating) {
        this.stopAnimation()
      }

      if (this.getCurrentStory === null) {
        this.setCurrentStory(this.getStories[this.lastIndex])
      } else {
        this.lastIndex++
        if (this.lastIndex > this.getStories.length - 1) {
          this.lastIndex = 0
        }
        this.setCurrentStory(this.getStories[this.lastIndex])
        this.searchedStory = this.getStories[this.lastIndex]
      }
    },
    ...mapActions('story',
      [
        'fetchStories'
      ]),
        ...mapActions('map',
      [
        'fetchUserPosition'
      ]),
      ...mapActions('explore',
      [
      'startAnimation',
      'stopAnimation',
      'setCurrentStory'
      ])
  },
  watch: {
    searchedStory (story) {
      if (story) {
        console.log('choose ', story)
        this.setCurrentStory(story)
      }
    }
  }
}
</script>

<style lang="sass" scoped>
.controls
  position: absolute
  width: 100%
  display: flex
  top: 50%
  flex-direction: row
  z-index: 1000
  justify-content: space-between
  pointer-events: none
  &_arrow_right
    margin-right: 20px
    img
      width: 60px
      height: 60px
      color: $pallete-5
      filter: invert(1) drop-shadow(2px 4px 6px black)
      cursor: pointer
      pointer-events: all

  &_arrow_left
    margin-right: 20px
    img
      transform: scaleX(-1)
      width: 60px
      height: 60px
      color: $pallete-5
      filter: invert(1) drop-shadow(2px 4px 6px black)
      cursor: pointer
      pointer-events: all

.explore
  z-index: 0
  height: 100%
  &_content
    height: 100%
    display: flex
    .map
      width: 100%
</style>
