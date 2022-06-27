<template>
  <div class="home">
    <div class='selected'>
        <h1 class='name'>
          {{ selectedLocationName }}
        </h1>
    </div>
    <div id="globe" v-touch:moving="onTouchMove" @touchmove="onTouchMove" @mousemove="onMouseMove" class="home_globe"></div>
  </div>
</template>
<script>
/* eslint-disable */
import { mapActions } from 'vuex'
import Globe from 'globe.gl';
import { mapGetters } from 'vuex'
import { getDistance } from 'geolib'
// eslint-disable-next-line
import { getDatabase, set, ref, onValue} from "firebase/database";
// import { db } from "@/main.js"
import throttle from 'lodash/throttle'
const THROTTLE_SPEED = 50
export default {
  name: 'home',
  components: {
  },
  computed: {
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
      selected: null
    }
  },
  beforeMount () {
    this.localIdentifier =  Math.floor(Math.random() * 10000)
        window.$ = window.jQuery = require('jquery');

  },
  mounted () {
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
    this.setupScene()
    process.nextTick(() => {
      // eslint-disable-next-line
      window.addEventListener('resize', (event) => {
        this.globe.width([event.target.innerWidth])
        this.globe.height([event.target.innerHeight])
      })
      this.observePov()
      this.observeID()
    })
    //ws://
    function addError(error) {
      console.log(error);
    }

    function onWebSocketOpen(ws) {
        console.log("on open");
        console.log(ws);
    }

    function onWebSocketMessage(evt) {
        console.log("on message:");
        console.log(evt.data);
    }

    function onWebSocketClose() {
        console.log("on close");
    }

    function onWebSocketError() {
        console.log("on error");
    }

    setTimeout(() => {
      this.JSONRPCClient = new $.JsonRpcClient({
        ajaxUrl: getDefaultPostURL(),
        socketUrl: getDefaultWebSocketURL(), // get a websocket for the localhost
        onmessage: onWebSocketMessage,
        onopen: onWebSocketOpen,
        onclose: onWebSocketClose,
        onerror: onWebSocketError
     });
    }, 500)
  },
  destroyed () {
  },
  methods: {
    setupScene () {
      const isMobile = this.mobileCheck()
      this.globe = new Globe({ antialias: false })
      this.globeEl = document.querySelector('#globe')
      this.globe(this.globeEl)
        .globeImageUrl('images/earth_bw.jpg')
        .bumpImageUrl('//unpkg.com/three-globe/example/img/earth-topology.png')
        .pointColor('color')
        .pointLat('north')
        .pointsMerge(true)
        .pointLng('west')
        .pointRadius(0.1)
        .pointAltitude('altitude')
        .onGlobeClick(this.onClickedLocation)
        .onGlobeRightClick(this.onClickedLocation)
        .backgroundColor('#FFFFFF')
        .atmosphereColor('#808080')
        .atmosphereAltitude(0.2)

        const scene = this.globe.scene()

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
        scene.children[2].intensity = 2
        if (isMobile) {
          this.globe.pointOfView({lat: 0, lng: 0, altitude: 5}, THROTTLE_SPEED)
        }
        // scene.children[2].position.set(0.5,0,0.5)// = 2
        let canvas = document.querySelector('canvas')
        canvas.addEventListener('touchmove', this.onTouchMove, false);
      }, 100)

    },
    onCompleteFetchCSV (csv) {
      this.locationData = csv.data
      
      this.locationData.forEach((item) => {
        item.color = '#00FF00'
        item.altitude = 0.001
      })
      
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
    onClickedLocation (point) {
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
    },
    onMouseMove() {
      this.currentPOV = this.globe.pointOfView()
      this.syncPOV(this.currentPOV.lat, this.currentPOV.lng, this.currentPOV.altitude)
    },
    onTouchMove() {
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
    observeID () {
      const db = getDatabase()
      const povRef = ref(db, 'selected/')
      onValue(povRef, (snapshot) => {
        const data = snapshot.val()
        if (data.id) {
          this.setSelectedByIndex(data.index)
        }
      })
    },
    setPov (data) {
      if (!data) {
        return
      }
      this.globe.pointOfView(data, THROTTLE_SPEED)
    },
    setSelectedByIndex (index) {
      if (!this.locationData) {
        return
      }
      for (let i = 0; i < this.locationData.length - 1; i++) {
        this.locationData[i].color = '#00FF00'
        this.locationData[i].altitude = 0.001
      }
      const clickedObject = this.locationData[index]
      this.selected = clickedObject

      clickedObject.color = 'red'
      clickedObject.altitude = 0.5

      this.locationData[index] = clickedObject

      this.globe.pointsData(this.locationData)
      if (this.JSONRPCClient) {
      this.JSONRPCClient.call('set-text',
          this.selected.id.toString(),
          function(result) {
            console.log(result)
          },
          function(error) {
              console.error(error())
          });
      } else {
        console.error('this.JSONRPCClient not defined')
      }

      setTimeout(() => {
        this.JSONRPCClient.call('get-text',
        null,
        function(result) {
            console.log('resultado?', result)
        },
        function(error) {
            addError(error);
        });
      }, 1200)
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
    ])
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
  margin: 10px
  pointer-events: none
.name
  font-family: Roboto
  font-size: 1em
  pointer-events: none

</style>
