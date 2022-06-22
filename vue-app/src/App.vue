<template>
<v-app id="app">
  <v-main>
    <v-container fluid fill-height fill-width class="ma-0 pa-0">
      <router-view class="view"></router-view>
    </v-container>
  </v-main>
</v-app>
</template>

<script>
import { mapActions } from 'vuex'

import {
} from '@/components'

export default {
  components: {
  },
  data () {
    return {
    }
  },
  created () {
    console.log('app created')
    // this.checkAuthentication()
  },
  mounted () {
  },
  updated () {
  },
  computed: {
  },
  methods: {
    ...mapActions('story',
    [
      'fetchStories'
    ]),
    ...mapActions('auth',
    [
      'checkAuthentication'
    ])
  },
  watch: {
  },
  beforeDestroy () {
  },
  name: 'app'
}
</script>

<style lang="sass">
#app
  font-family: 'Heebo', sans-serif
  -webkit-font-smoothing: antialiased
  -moz-osx-font-smoothing: grayscale
  text-align: center
  color: #2c3e50
  height: 100vh
  width: 100vw
  overflow-y: hidden
  overflow-x: hidden
  .view
    width: 100%
    height: 100%

#nav
  padding: 30px
  a
    font-weight: bold
    color: #2c3e50
    &.router-link-exact-active
      color: #42b983
</style>
