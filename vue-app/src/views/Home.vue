<template>
  <div class="home">
    <div class='selected'>
        <h1 class='name'>
          {{ nameToShow }}
        </h1>
    </div>
    <div id="globe" @touchmove="onTouchMove" @mousemove="onMouseMove" class="home_globe"></div>
    <div class='send' v-if="isMobile" >
      <a class='send_button' @click="setChosen" @click.native="setChosen">
        LISTEN
      </a>
    </div>
    <div v-if="!isMobile" class="qrcode">
      <img class="qrcodeimage" src="/images/qrcodesite.png"/>
    </div>
  </div>
</template>
<script>
/* eslint-disable */
import { mapActions } from 'vuex'
import Globe from 'globe.gl';
import { mapGetters } from 'vuex'
import { getDistance } from 'geolib'
import * as THREE from 'three'
// eslint-disable-next-line
import { getDatabase, set,query, ref, get, onValue, child, onChildAdded} from "firebase/database";
// import { db } from "@/main.js"
import throttle from 'lodash/throttle'
const THROTTLE_SPEED = 50
export default {
  name: 'home',
  components: {
  },
  computed: {
    isMobile () {
      return this.mobileCheck()
    },
    nameToShow () {
      if (this.isMobile) {
        return this.selectedLocationName
      }
      return this.chosenLocationName
    },
    chosenLocationName () {
      if (this.chosen) {
        return this.locationData[this.chosen.index].name
      }
      return ""
    },
    selectedLocationName () {
      if (this.selected) {
        return this.selected.name
      }
      return ""
    },
    ...mapGetters('story',
      [
      ])
  },
  data () {
    return {
      isOpen: false,
      JSONRPCClient: null,
      localIdentifier: null,
      globe: null,
      globeEl: null,
      locationData: null,
      currentPOV: {
        lat: 0,
        lng: 0,
        altitude: 2.5
      },
      selected: null,
      chosen: null,
      checkInterval: null
    }
  },
  beforeMount () {
    this.localIdentifier =  Math.floor(Math.random() * 10000)
        window.$ = window.jQuery = require('jquery');
  },
  mounted () {
      console.log('v0.13')
      if (!this.mobileCheck()) {
        let script2 = document.createElement('script')
        script2.setAttribute('src', '/js/jquery.jsonrpcclient.js')
        document.head.appendChild(script2)

        let script3 = document.createElement('script')
        script3.setAttribute('src', '/js/jquery.json.js')
        document.head.appendChild(script3)

        let script4 = document.createElement('script')
        script4.setAttribute('src', '/js/prism.js')
        document.head.appendChild(script4)

        let script5 = document.createElement('script')
        script5.setAttribute('src', '/js/ofxUtils.js')
        document.head.appendChild(script5)

        script5.onload = () => {
            setTimeout(() => {

            console.log('scripted loaded')
            function addError(error) {
              console.log(error);
            }

            function onWebSocketMessage(evt) {
                console.log("on message:");
                console.log(evt.data);
            }

            function onWebSocketClose() {
                console.error("on close socket OF");
            }

            function onWebSocketError() {
                console.error("on error connecting to socket of OF");
            }

            if (!this.mobileCheck()) {
                console.log('set to connect to OF')
                this.JSONRPCClient = new $.JsonRpcClient({
                  ajaxUrl: getDefaultPostURL(),
                  socketUrl: getDefaultWebSocketURL(), // get a websocket for the localhost
                  onmessage: onWebSocketMessage,
                  onopen: this.onWebSocketOpen,
                  onclose: onWebSocketClose,
                  onerror: onWebSocketError
                })
                console.log('json maluco ', this.JSONRPCClient)
            }
          })
        }
    }
    this.setupScene()
    setTimeout(() => {

      // eslint-disable-next-line
      window.addEventListener('resize', (event) => {
        this.globe.width([event.target.innerWidth])
        this.globe.height([event.target.innerHeight])
      })

      process.nextTick(() => {
        console.log('observe')
        this.observePov()
        this.observeSelected()
        this.observeChosen()
        if (!this.mobileCheck()) {
          this.checkOFFungus()
        }
        //setTimeout(() => {
        //  this.generateArcs()
        //}, 800)
      })
    }, 500)
    //ws://
  },
  destroyed () {
  },
  methods: {
    checkOFFungus () {
      this.checkInterval = setInterval(() => {
        console.log('chamou')
        if (this.JSONRPCClient) {
          this.JSONRPCClient.call('get-text',
          null,
          (result)=> {
              console.log('resultado?', result)
              if (this.chosen && this.chosen.id.toString() !== result.toString()) {
                console.warn("wrong loca, set loca")
                this.setChosenByID(result)
              } else {
                console.log('good location')
              }
          },
          function(error) {
              addError(error);
          })
        } else {
          console.warn('no json RPC')
        }
      }, 5000)
    },
     onWebSocketOpen(ws) {
        console.log("on open connection to of");
        this.isOpen = true
        console.log(ws);
    },
    setupScene () {
      const isMobile = this.mobileCheck()
      console.log('isMobile', isMobile)
      this.globe = new Globe({ antialias: false, precision: 'mediump', powerPreference: 'high-performance', depth: false})
      this.globeEl = document.querySelector('#globe')
      this.globe(this.globeEl)
        .globeImageUrl('images/earth_bw.jpg')
        .bumpImageUrl('//unpkg.com/three-globe/example/img/earth-topology.png')
        .pointColor('color')
        .pointLat('north')
        .pointsMerge(true)
        .pointLng('west')
        .pointRadius(0.1)
        .pointResolution(3)
        .pointAltitude('altitude')
        .onGlobeClick(this.searchLocation)
        .onGlobeRightClick(this.searchLocation)
        .backgroundColor('#000000')
        .showAtmosphere(false)
        .arcColor('color')

        const scene = this.globe.scene()
        console.log('configurou!')
      this.$papa.parse('/locationsOnlyImage.csv', {
        download: true,
        header: true,
        complete: this.onCompleteFetchCSV
      })
      /*
      this.$papa.parse('/observations.csv', {
        download: true,
        header: true,
        complete: this.onCompleteFetchObservations
      })
      */

      setTimeout(() => {
        console.log('chamou')
        if (!isMobile) {
          this.globe.enablePointerInteraction(false)
        }
        scene.children[2].intensity = 2
        if (isMobile) {
          this.globe.pointOfView({lat: 0, lng: 0, altitude: 5}, THROTTLE_SPEED)
        }
        // scene.children[2].position.set(0.5,0,0.5)// = 2
                console.log('addou')

        let canvas = document.querySelector('canvas')
        canvas.addEventListener('touchmove', this.onTouchMove, false);

        // this.globe.renderer().setPixelRatio(0.5)
      }, 100)

    },
    generateArcs() {
      console.log("GENERATE ARCS")
      const arcs = []
        const db = getDatabase()
        const dbRef = ref(db);
        get(child(dbRef, `chosen/`)).then((snapshot) => {
          console.log(snapshot)
            if (snapshot.exists()) {
              const data = snapshot.val()
              const arrChosen = Object.values(data)
              for(let i = 0; i < arrChosen.length; i++) {
                if (i !== 0) {
                  const arc = {
                    startLat: this.locationData[arrChosen[i-1].index].north,
                    startLng: this.locationData[arrChosen[i-1].index].west,
                    endLat: this.locationData[arrChosen[i].index].north,
                    endLng: this.locationData[arrChosen[i].index].west,
                    color: 'white'
                  }
                  arcs.push(arc)
                }
              }
              console.log('arcs', arcs)
              this.globe.arcsData(arcs)
            }
          })
    },
    onCompleteFetchCSV (csv) {
      console.log('location', csv)

      this.locationData = csv.data
      
      this.locationData.forEach((item) => {
        item.color = '#00FF00'
        item.altitude = 0.001
      })
      
      if (this.mobileCheck()) {
        return
      }
      
      this.globe.pointsData(this.locationData)
    },
    /*
    onCompleteFetchObservations (csv) {
      // filter
      const observations = csv.data
      console.log(observations)
      const observationsWithImage = observations.filter((observation) => {
        return observation.thumb_image_id !== "NULL"
      })
      console.log(observationsWithImage)
      const ids = []
      observationsWithImage.forEach((ob) => {
        ids.push(ob.location_id)
      })
    console.log('locationData', this.locationData)
    const locationsOnlyImage = this.locationData.filter((location) => {
      return (ids.indexOf(location.id) !== -1)
    })
    console.log('locationsOnlyImage', locationsOnlyImage)

    const exportCSV = this.$papa.unparse(locationsOnlyImage);

    var csvData = new Blob([exportCSV], {type: 'text/csv;charset=utf-8;'});

    var csvURL = window.URL.createObjectURL(csvData);

    var testLink = document.createElement('a');
    testLink.href = csvURL;
    testLink.setAttribute('test', 'test.csv');
    testLink.click();
    },
    */
    searchLocation: throttle(function (point) {
      if (!this.locationData) {
        return
      }
      const distances = []

      for (let i = 0; i < this.locationData.length - 1; i++) {
        const dist = getDistance(
            { latitude: point.lat, longitude: point.lng },
            { latitude: this.locationData[i].north, longitude: this.locationData[i].west }
        )
        distances.push(dist)
      }
      
      let minDist = Math.min(...distances)
     const minIndex = distances.indexOf(minDist)
     const clickedObject = this.locationData[minIndex]
      const db = getDatabase()
      set(ref(db, 'selected/'), {
        id: clickedObject.id,
        index: minIndex
      })
    }, 100),
    onTouchMove () {
      this.currentPOV = this.globe.pointOfView()
      this.syncPOV(this.currentPOV.lat, this.currentPOV.lng, this.currentPOV.altitude)
      this.searchLocation({lat:  this.currentPOV.lat, lng:  this.currentPOV.lng})
    },
    onMouseMove() {
      this.currentPOV = this.globe.pointOfView()
      this.syncPOV(this.currentPOV.lat, this.currentPOV.lng, this.currentPOV.altitude)
    },
    syncPOV: throttle(function (lat, lng, altitude) {
      const db = getDatabase()
      set(ref(db, 'pov/'), {
        lat: lat,
        lng: lng,
        altitude: altitude,
        id: this.localIdentifier
      })
    }, THROTTLE_SPEED),
    observePov () {
      const db = getDatabase()
      const povRef = ref(db, 'pov/')
      onValue(povRef, (snapshot) => {
        const data = snapshot.val()
        if (data.id !== this.localIdentifier) {
          this.setPov(data)
        }
      })
    },
    observeSelected () {
      console.log('observo select')
      const db = getDatabase()
      const povRef = ref(db, 'selected/')
      onValue(povRef, (snapshot) => {
        const data = snapshot.val()
        if (data && data.id) {
          this.setSelectedByIndex(data.index)
        }
      })
    },
    observeChosen () {
      let length = 0
      const db = getDatabase()
      const dbRef = ref(db);
       get(child(dbRef, `chosen/`)).then((snapshot) => {
          console.log(snapshot)
          if (snapshot.exists()) {
            console.log(snapshot.val());
            const data = snapshot.val()

            length = data.length
            console.log('length', length)
              console.log('observe chosen')
              const chosenRef = ref(db, 'chosen/')
              onChildAdded(query(chosenRef), (snapshot) => {
                    const data = snapshot.val()
                      console.log('on child added', data)
                    if (data) {
                      console.log('index', snapshot.key)
                      if (snapshot.key > length - 2) {
                        this.play(data)
                      }
                    } else {
                      console.log('no data')
                    }
                  })
          }
        })
    },
    setPov (data) {
      if (!data) {
        return
      }
      if (!this.isMobile) {
        if (data.altitude < 2.2) data.altitude = 2.2
          data.altitude = data.altitude + 0.2
      }
      this.globe.pointOfView(data, THROTTLE_SPEED)
    },
    setSelectedByIndex (index) {
      if (!this.locationData || !this.globe) {
        return
      }
      for (let i = 0; i < this.locationData.length - 1; i++) {
        this.locationData[i].color = '#00FF00'
        this.locationData[i].altitude = 0.001
      }
      const clickedObject = this.locationData[index]
      clickedObject.index = index
      this.selected = clickedObject

      clickedObject.color = '#00FF00'
      clickedObject.altitude = 0.5

      if (this.mobileCheck()) {
        this.locationData[index] = clickedObject
        this.globe.pointsData([clickedObject])
      }
    },
    setChosenByID (id) { // ONLY DESKTOP
      if (!this.locationData || !this.globe) {
        return
      }
      id = id.toString()
      console.log('set sel', id)
      let selected = null
      let index = 0
      for (let i = 0; i < this.locationData.length - 1; i++) {
        this.locationData[i].color = '#00FF00'
        this.locationData[i].altitude = 0.001
        if (this.locationData[i].id === id) {
          selected = this.locationData[i]
          index = i
        }
      }
      console.log('set sel', selected)

      const clickedObject = selected
      clickedObject.index = index
      this.selected = clickedObject

      clickedObject.color = '#00FF00'
      clickedObject.altitude = 0.5

      this.chosen = this.selected

      this.locationData[index] = clickedObject
      this.globe.pointsData(this.locationData)
      const dataLatLng = {
        lat: clickedObject.north, lng: clickedObject.west, altitude: this.currentPOV.altitude
      }
      this.setPov(dataLatLng)
      
    },
    play (chosen) {
      if (!this.isOpen) {
        console.warn("SOCKET NOT OPEN YET")
      }
      this.chosen = this.locationData[chosen.index]
      
      if (this.JSONRPCClient && !this.mobileCheck()) {
        this.chosen = this.locationData[chosen.index]
        this.globe.pointsData(this.locationData)

        console.log('send to play', chosen.id)
        console.log('send to play index', chosen.index)

      this.JSONRPCClient.call('set-text',
          chosen.id.toString(),
          function(result) {
            console.log(result)
          },
          function(error) {
              console.error(error())
          });
      } else {
        console.warn('this.JSONRPCClient not defined or socket not open')
      }
      if (!this.mobileCheck()) {
        this.generateArcs()
      }
    },
    mobileCheck () {
      let check = false; // eslint-disable-next-line
      (function(a){if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino/i.test(a)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0,4))) check = true;})(navigator.userAgent||navigator.vendor||window.opera);
      return check;
    },
    ...mapActions('map', [
      'fetchUserPosition'
    ]),
    ...mapActions('story', [
      'fetchStories'
    ]),
    setChosen () {
        if (!this.selected)
        {
          return
        }
          console.log('set chosen clicked')
          const db = getDatabase()
          const dbRef = ref(db);
          get(child(dbRef, `chosen/`)).then((snapshot) => {
          console.log(snapshot)
          if (snapshot.exists()) {
            console.log(snapshot.val());
            const data = snapshot.val()

            console.log('length', data.length)
            console.log('data', data)
            data[data.length] = {
              id: this.selected.id,
              index: this.selected.index
            }
            console.log('data after', data)
            set(ref(db, 'chosen/'), {
              ...data
            })
            
          } else {
            console.log("No data available");
            console.log(this.selected)
            set(ref(db, 'chosen/'), {
              0: {
                id: this.selected.id,
                index: this.selected.index
              }
            })
          }
        }).catch((error) => {
          console.error(error);
        });

    }
  }
}
</script>
<style lang='sass' scoped>
.home
  &_globe
    width: 100%
    height: 100%
    cursor: move
    cursor: grab
    cursor: -moz-grab
    cursor: -webkit-grab
    :active
      cursor: grabbing
      cursor: -moz-grabbing
      cursor: -webkit-grabbing

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


.selected
  z-index: 1000
  position: fixed
  top: 0
  height: 60px
  display: flex
  width: 100vw
  padding: 10px
  pointer-events: none
.name
  font-family: Courier
  font-size: 50px
  pointer-events: none
  color: white
  text-shadow: 2px 2px black
  @media (max-width:600px)
    font-size: 25px
.send
  position: fixed
  bottom: 20px
  width: 100%
  height: 40px
  display: flex
  justify-content: center
  color: white

.send_button
  font-size: 50px
  font-family: Courier
  z-index: 999999 !important
  pointer-events: all !important
  width: 100px
  height: 100%
  text-align: center
  display: flex
  justify-content: center
  align-items: center
  vertical-align: middle
  color: white
  text-shadow: 2px 2px black

.qrcode
  position: fixed
  bottom: 0
  height: 200px
  display: flex
  justify-content: flex-end
  width: 100vw

.qrcodeimage

</style>
