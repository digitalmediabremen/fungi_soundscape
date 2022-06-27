<template>
  <header class="topbar">
    <div class="wrap">
      <a class="wrap_back" v-if="getMeta.back" @click="goBack">
        <img src="static/icons/backarrow-icon.svg" />
      </a>
      <router-link class="wrap_logo" v-if="!getMeta.back" to="/" tag="a">choral-mycelia</router-link>
      <div class="controls">
        <!--
          <div
            class="controls_btn"
            :class="{ 'controls_btn--selected': $route.path === '/create' }"
          >Create</div>
        -->
        <div
          v-if='!isAuthenticated'
          @click="$router.push('/register')"
          class="controls_btn"
          :class="{ 'controls_btn--selected': $route.path === '/register' }"
        >Register</div>
        <v-btn
          class="ml-9 align-self-center"
          small
          dark
          v-if="isAuthenticated"
          :loading="logginout"
          tile
          color="black"
          @click="submitLogout"
        >
          Logout
        </v-btn>
        <div
          @click="$router.push('/explore')"
          class="controls_btn"
          :class="{ 'controls_btn--selected': $route.path === '/explore' }"
        >Explore</div>
        <div
          @click="$router.push('/')"
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

import {
} from '@/components/atoms'

export default {
  components: {
  },
  mounted () {
  },
  updated () {
  },
  data () {
    return {
      logginout: false
    }
  },
  computed: {
    ...mapGetters('auth', [
      'isAuthenticated'
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
    submitLogout () {
      this.logginout = true
      this.logout().then(() => {
        this.logginout = false
      })
    },
    goBack: function () {
      this.$router.push(-1)
    },
    ...mapActions('map', [
      'fetchCurrentPosition'
    ]),
    ...mapActions('auth', [
      'logout'
    ])
  },
  watch: {
  }
}
</script>

<style lang="sass" scoped>
.topbar
  position: fixed
  width: 90%
  background: $white-1
  z-index: 10
  height: 50px
  .wrap
    justify-content: space-between
    display: flex
    flex-direction: row
    &_logo
      font-weight: 1000
      justify-content: center
      align-self: center
      width: 100px
      color: black
      img
        max-width: 100%
        height: auto
    &_back
      color: black
      width: 15px
      padding: 2px

    .controls
      width: 50%
      padding-right: 20px
      height: 50px
      display: flex
      align-content: right
      justify-content: right
      flex-direction: row-reverse

      &_btn
        opacity: 0.4
        cursor: pointer
        width: 100px
        height: 100%
        margin-left: 1.2em
        font-weight: 800
        font-size: 15px
        color: black
        display: flex
        justify-content: center
        flex-direction: column
        text-align: center

        &--selected
          opacity: 1

        &:hover:not(.controls_btn--selected)
          color: red
</style>
