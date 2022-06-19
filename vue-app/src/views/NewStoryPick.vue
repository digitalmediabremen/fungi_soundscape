<template>
  <v-col class='newstorypick fill-height fill-width' align="center" justify="center" align-content="center">
    <v-app-bar
      color="black accent-4"
      dense
      dark
    >
    <v-toolbar-title>{{ getCreatingStory ? getCreatingStory.title : 'Creating' }}</v-toolbar-title>

    </v-app-bar>
    <MapBox
      :creatingStory="true"
      class="map"
    />
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
        'getCreatingStory'
      ])
  },
  data: () => ({
  }),

  methods: {
    ...mapActions('auth', [
      'login',
      'getUser'
    ]),
    ...mapActions('story', [
      'storeCreatingStory'
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
  width: 100% !important
  height: 100% !important
</style>