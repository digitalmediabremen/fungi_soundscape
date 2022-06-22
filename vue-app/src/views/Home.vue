<template>
  <div class="home">
    <div id="globe" class="home_globe"></div>
  </div>
</template>

<script>
import { mapActions } from 'vuex'
import Globe from 'globe.gl';
import { mapGetters } from 'vuex'
import { getDistance } from 'geolib'

export default {
  name: 'home',
  components: {
  },
  computed: {
    ...mapGetters('story',
      [
        'getStories'
      ])
  },
  data () {
    return {
      globe: null,
      locationData: null
    }
  },
  mounted () {
    this.setupScene()
  },
  destroyed () {
    window.removeEventListener('resize', this.onWindowResize)
  },
  methods: {
    setupScene () {
      this.globe = new Globe()
      this.globe(document.querySelector('#globe'))
        .globeImageUrl('images/earth_bw.jpg')
        .bumpImageUrl('//unpkg.com/three-globe/example/img/earth-topology.png')
        .pointColor(() => '#00FF00')
        .pointLat('north')
        .pointsMerge(true)
        .pointLng('west')
        .pointRadius(0.1)
        .pointAltitude(0.001)
        .onGlobeClick(this.onClickedLocation)
        .backgroundColor('#FFFFFF')
        .atmosphereColor('#808080')
        .atmosphereAltitude(0.2)

        console.log(this.globe.scene())
        const scene = this.globe.scene()
        console.log(scene.children)
      this.$papa.parse('/locations.csv', {
        download: true,
        header: true,
        complete: this.onCompleteFetchCSV
      })

      setTimeout(() => {
        scene.children[2].intensity = 2
        // scene.children[2].position.set(0.5,0,0.5)// = 2
      }, 100)

    },
    onCompleteFetchCSV (locations) {
      console.log(locations.data)
      this.locationData = locations.data
      this.globe.pointsData(this.locationData)
    },
    onClickedLocation (point, event) {
      console.log(point)
      console.log(event)
      const distances = []

      for (let i = 0; i < this.locationData.length - 1; i++) {
        const dist = getDistance(
            { latitude: point.lat, longitude: point.lng },
            { latitude: this.locationData[i].north, longitude: this.locationData[i].west }
        )
        distances.push(dist)
      }
      
      let minDist = Math.min(...distances)
      console.log('minDist', minDist)

     const clickedObject = this.locationData[distances.indexOf(minDist)]
     console.log(clickedObject)
    },
    ...mapActions('map', [
      'fetchUserPosition'
    ]),
    ...mapActions('story', [
      'fetchStories'
    ])
  }
}
</script>
<style lang='sass' scoped>
.home
  &_globe
    width: 100%
    height: 100%
  &_message
    position: absolute
    display: flex
    justify-content: center
    flex-direction: column
    align-items: center
    width: 400px
    height: 100px
    left: 50%
    top: calc(50% - 50px)
    -webkit-transform: translate(-50%, 0)
    transform: translate(-50%, 0)
    cursor: pointer
    &_text
      color: $pallete-5
      align-self: center
      font-weight: 800
      font-size: 22px
      text-transform: uppercase
    &_interaction
      color: red
      align-self: center
      font-weight: 800
      margin: 0 auto
      font-size: 30px
      text-transform: uppercase

.fade-enter-active, .fade-leave-active
  transition: opacity .2s
  transition-delay: 0s !important

.fade-enter, .fade-leave-to /* .fade-leave-active below version 2.1.8 */
  opacity: 0

.fadedelay-enter-active, .fadedelay-leave-active
  transition-delay: 0.2s !important
  position: absolute
  transition: opacity .2s

.fadedelay-enter, .fadedelay-leave-to /* .fade-leave-active below version 2.1.8 */
  opacity: 0
  transition-delay: 0s !important
</style>
