<template>
  <div class="pill">
    <div class="pill_line" />
    <div class="pill_info">
      <h3 class="pill_info_title">
        {{ story.title }}
      </h3>
      <a
        class="pill_info_description"
        @click="showStory"
      >{{ story.description }}</a>
    </div>
  </div>
</template>

<script>
export default {
  props: {
    story: {
      type: Object,
      default: null
    }
  },
  data () {
    return {
      ready: false
    }
  },
  mounted () {
  },
  updated () {
  },
  methods: {
    showStory () {
      this.$router.push('/stories/' + this.story.id)
    }
  }
}
</script>
<style lang="sass">
.mapboxgl-popup-tip
  width: 0
  padding-top: 5px
  height: 100px !important
  border: 1px solid black !important
  z-index: 1

.mapboxgl-popup-content
  background: $white-1
  padding: 1px !important
  height: fit-content

.mapboxgl-popup-close-button
  color: $white-1 !important
  font-size: 144% !important
</style>
<style lang="sass" scoped>
.pill
  background: transparent
  height: auto
  min-height: 30px
  &_info
    width: 200px
    height: auto
    cursor: pointer
    background: black
    &_title
      font-weight: 900
      color: white
    &_description
      font-weight: 300
      color: white
      margin-bottom: 2px
</style>
