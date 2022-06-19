<template>
  <div class="home">
    <div id="globe" class="home_globe"></div>
    <div class="home_message" @mouseover="hoverMessage = true" @mouseleave="hoverMessage = false">
      <transition-group name="fade" mode="out-in">
        <p
          v-for="(message) in currentMessages"
          :key="message"
          v-show="!hoverMessage"
          class="home_message_text"
        >{{message}}</p>
      </transition-group>
      <transition name="fadedelay" mode="out-in">
        <p
          v-if="hoverMessage"
          class="home_message_interaction"
          @click="$router.push('/explore')"
        >Explore</p>
      </transition>
    </div>
  </div>
</template>

<script>
import { mapActions } from 'vuex'
import * as THREE from 'three';
import ThreeGlobe from 'three-globe';
// import { TrackballControls } from 'three/examples/jsm/controls/TrackballControls.js';
import { mapGetters } from 'vuex'

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
      renderer: null,
      camera: null,
      messages: [
        'Oil Spills',
        'Deforestation',
        'Illegal Mining',
        'Fracking',
        'Wild Animal Trafficking',
        'Toxic Waste',
        'Aquifer Contamination',
        'Greenhouse Gas Emissions'
      ],
      currentMessages: [],
      messageAnimation: null,
      hoverMessage: false
    }
  },
  mounted () {
    this.fetchStories().then(() => {
      this.setupScene()
      this.animateMessage()
    })
  },
  destroyed () {
    clearInterval(this.messageAnimation)
    window.removeEventListener('resize', this.onWindowResize)
  },
  methods: {
    animateMessage () {
      console.log("created animation")
      this.messageAnimation = setInterval(() => {
        let index = this.messages.indexOf(this.currentMessages[0])
        index++
        if (index > this.messages.length - 1) {
          index = 0
        }
        let newMessage = this.messages[index]
        this.currentMessages = []
        this.currentMessages.push(newMessage)
      }, 2000)
    },
    async setupScene () {
      await window.THREE

      let pathData = []

      this.getStories.forEach(story => {
        let path = [...story.geometry.coordinates.map((coord) => {
          return [coord.lat, coord.lng, 0.1]
        })]
        pathData.push(path)
      })

      const Globe = new ThreeGlobe()
        .globeImageUrl('images/earth_bw.jpg')
        .bumpImageUrl('//unpkg.com/three-globe/example/img/earth-topology.png')
        .pathsData(pathData)
        .pathColor(() => 'rgba(255,0,0,1)')

      // setTimeout(() => {
        // gData.forEach(d => d.size = Math.random())
        // Globe.pointsData(gData)
      // }, 4000)

      // custom globe material
      const globeMaterial = Globe.globeMaterial();
      globeMaterial.bumpScale = 10;
      new THREE.TextureLoader().load('//unpkg.com/three-globe/example/img/earth-water.png', texture => {
        globeMaterial.specularMap = texture;
        globeMaterial.specular = new THREE.Color('#ffffff');
        globeMaterial.shininess = 15;
      })

      // Setup renderer
      const width = document.querySelector('#globe').clientWidth
      const renderer = this.renderer = new THREE.WebGLRenderer()
      renderer.setSize(width, window.innerHeight)
      document.getElementById('globe').appendChild(renderer.domElement)

      // Setup scene
      const scene = new THREE.Scene()
      scene.add(Globe)
      scene.add(new THREE.AmbientLight(0xbbbbbb))
      scene.add(new THREE.DirectionalLight(0xffffff, 0.5))
      scene.background = new THREE.Color( 0xffffff);

      // Setup camera
      const camera = this.camera = new THREE.PerspectiveCamera()
      camera.aspect = (width)/window.innerHeight
      camera.updateProjectionMatrix()
      camera.position.z = 500

      // Add camera controls
      /*
      const tbControls = new TrackballControls(camera, renderer.domElement)
      tbControls.minDistance = 101
      tbControls.rotateSpeed = 5
      tbControls.zoomSpeed = 0.8
      */

      // Kick-off renderer
      let r = 0
      let animate = () => { // IIFE
        // Frame cycle
        // tbControls.update()
        renderer.render(scene, camera)
        r=r+0.005
        Globe.rotation.set(0, r, 0)
        requestAnimationFrame(animate)
      }
      window.addEventListener('resize', this.onWindowResize)
      animate()
    },
    onWindowResize () {
      console.log('resize')
      const width = document.querySelector('#globe').clientWidth
      this.camera.aspect = width/ window.innerHeight
      this.camera.updateProjectionMatrix()
      this.renderer.setSize(width, window.innerHeight)
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
<style lang='sass'>
canvas
  width: 100% !important
</style>
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
