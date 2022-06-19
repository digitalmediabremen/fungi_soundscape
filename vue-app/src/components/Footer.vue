<template>
    <header class="footer">
      <div class="wrap">
        <!--
        <a class="wrap_back" v-if='getMeta.back' @click="goBack">
          <img src="static/icons/backarrow-icon.svg">
        </a>
        <router-link class="wrap_logo" v-if='!getMeta.back' to="/" tag="a">
        LOGO
        </router-link>
        -->
        <div class="controls">
          <!--
          <div
            class="controls_btn"
            :class="{ 'controls_btn--selected': $route.path === '/create' }"
          >Create</div>
          -->
          <div @click="$router.push('/explore')"
            class="controls_btn"
            :class="{ 'controls_btn--selected': $route.path === '/explore' }"
          >Explore</div>
          <div @click="$router.push('/')"
            class="controls_btn"
            :class="{ 'controls_btn--selected': $route.path === '/' }"
          >Home</div>
        </div>
      </div>
    </header>
</template>

<script>

import {
  mapGetters,
  mapActions
} from 'vuex'

export default {
  components: {
  },
  mounted () {
  },
  updated () {
  },
  data () {
    return {
    }
  },
  computed: {
    ...mapGetters([
      'isModalOpen'
    ]),
    getMeta () {
      if (this.$route.meta.top) {
        if (this.$route.meta.top.show) {
          return this.$route.meta.top
        }
        return false
      }
      return false
    }
  },
  methods: {
    goBack: function () {
      this.$router.push(-1)
    },
    onClickMap () {
      console.log('push map')
      this.$router.push('/map')
    },
    ...mapActions('map', [
      'fetchUserPosition'
    ])
  },
  watch: {
  }
}
</script>

<style lang="sass" scoped>
$BtnSizeResetAuto: auto !important
$BtnSizeResetNone: none !important

.footer
  position: absolute
  bottom: 0
  width: 100vw
  background: #025d9e
  z-index: 10
  height: 50px
  .wrap
    justify-content: space-between
    display: flex
    flex-direction: row
    width: 100%
    &_logo
      justify-content: center
      align-self: center
      width: 50px
      color: white
      img
        max-width: 100%
        height: auto
    &_back
      color: white
      width: 15px
      padding: 2px

    .controls
      width: 100%
      height: 50px
      display: flex
      align-content: center
      justify-content: center
      flex-direction: row-reverse
      &_btn
        opacity: 0.8
        cursor: pointer
        width: 100px
        height: 100%
        margin-left: 5px
        font-weight: 800
        font-size: 16px
        color: white
        display: flex
        justify-content: center
        flex-direction: column
        text-align: center

        &--selected
          opacity: 1

.btn_topbar
  margin-right: 0
  width: $BtnSizeResetAuto
  height: $BtnSizeResetAuto
  max-width: $BtnSizeResetNone
  min-width: $BtnSizeResetAuto
  max-height: $BtnSizeResetNone
  min-height: $BtnSizeResetAuto

</style>
