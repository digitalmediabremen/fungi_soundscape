<template>
  <div class="map">
    <MglMap
      :accessToken="accessToken"
      mapStyle="mapbox://styles/antoniohof/ckc7p8lpj0tlh1ileaec3bel4?optimize=true"

      :center="this.initialCenter"
      :zoom="this.initialZoom"
      @load="onMapLoaded"
      @zoomend="onZoomChanged"
      @dragend="onCenterChanged"
    >
      <MglMarker
        @click="onClickedStory(story)"
        v-for="(story, index) in storiesAdded"
        :key="index"
        :coordinates="getCenterLayer(story)"
      >
        <div :id="'marker_' + story.id" slot="marker" class="marker" />
        <MglPopup
          :keepInView="true"
          v-if="getCurrentStory && getCurrentStory.id === story.id"
        >
          <!--<PodcastPill :story="story"></PodcastPill>-->
          <v-card
            raised
            tile
            elevation="12"
            max-width="300"
            width="300"
            class="mx-auto p-5 storycard"
          >
            <v-list-item>
              <!-- <v-list-item-avatar color="grey"></v-list-item-avatar> -->
              <v-list-item-content class='storycard_body'>
                <v-list-item-title class="storycard_title text-wrap">{{ story.title }}</v-list-item-title>
                <v-list-item-subtitle>{{ story.author }}</v-list-item-subtitle>
              </v-list-item-content>
            </v-list-item>

            <v-img
              once
              :src="story.thumbnail.downloadUrl"
              height="194"
            ></v-img>

            <v-card-text class='storycard_description' max-height="200">
              {{ story.description }}
            </v-card-text>

            <v-card-actions>
              <v-btn
                @click="$router.push('/story/' + story.id)"
                text
                dark
                color="black accent-4"
              >
                Read More
              </v-btn>
              <v-spacer></v-spacer>
              <v-btn icon v-clipboard="getLink(story)">
                <v-icon>mdi-link-variant</v-icon>
              </v-btn>
            </v-card-actions>
          </v-card>
        </MglPopup>
      </MglMarker>
      <MglGeojsonLayer
        v-for="story in storiesAdded"
        :key="story.id"
        :sourceId="story.id"
        :layerId="story.id"
        :layer="layerInfo(story.id)"
        @click="onClickedStory(story)"
      ></MglGeojsonLayer>
    </MglMap>
    <div v-if="creatingStory" class="calculation-box">
      <p>Use the draw tool on the top left to mark the area of the incident</p>
      <div id="calculated-area"></div>
    </div>
    <v-btn v-if="creatingStory" @click="onClickedCreate" class="createbutton ma-2" color="black" dark>Create
      <v-icon dark right>mdi-checkbox-marked-circle</v-icon>
    </v-btn>
  </div>
</template>
<script>
var MapboxDraw = require('@mapbox/mapbox-gl-draw');
import { mapGetters, mapActions } from 'vuex'
import { MglMap, MglGeojsonLayer, MglPopup, MglMarker  } from "vue-mapbox";
// import polygonStyles from '@/assets/js/polygonStyles.js'
import {
  // PodcastPill
} from '@/components/atoms'
export default {
  components: {
    MglMap,
    MglGeojsonLayer,
    MglPopup,
    MglMarker
    // PodcastPill
  },
  props: {
    stories: {
      type: Array,
      required: false,
      default: () => []
    },
    creatingStory: {
      type: Boolean,
      required: false,
      default: false
    },
    locked: {
      type: Boolean,
      require: false,
      default: false
    }
  },
  beforeMount () {
    this.initialCenter = [this.getLastLocation.lng, this.getLastLocation.lat]
    this.initialZoom = this.getLastLocation.zoom
  },
  mounted () {
  },
  data () {
    return {
      initialCenter: [],
      initialZoom: 2,
      mapbox: null,
      lastSelected: -1,
      accessToken: process.env.VUE_APP_MAPBOX_KEY,
      currentDraw: null,
      storiesAdded: [],
      lastClickedID: null,
      readyToCreate: false
    }
  },
  computed: {
    ...mapGetters('map', [
      'getLastLocation'
    ]),
    ...mapGetters('explore', [
      'getIsAnimating',
      'getCurrentStory'
    ]),
    ...mapGetters('story', [
      'getCreatingStory'
    ])
  },
  methods: {
    getLink (s) {
      return window.location.origin + '/story/' + s.id
    },
    async onMapLoaded(evt) {
      this.mapbox = evt.map
      // Here we cathing 'load' map event
      // const asyncActions = event.component.actions

      // await asyncActions.setCenter([this.getLastLocation.lng, this.getLastLocation.lat])
      // await asyncActions.setZoom(this.getLastLocation.zoom)

      console.log("map loaded")

      /*
      this.mapbox.loadImage('/images/blood.png', (err, img) => {
        if (err) {
          console.error(err)
        }
        this.mapbox.addImage('pattern', img)
      })
      */

      if (this.locked) {
        console.log('locked map')
        let story = this.stories[0]
        console.log(story)

        setTimeout(() => {
          this.mapbox.jumpTo({
            center: [this.getCenterLayer(story)[0], this.getCenterLayer(story)[1]],
            essential: true,
            zoom: story.zoom ? story.zoom : 4
          })
        }, 200)
      }


      // add draw controls
      // draw mode
      if (this.creatingStory) {
        this.currentDraw = new MapboxDraw({
          displayControlsDefault: false,
          controls: {
              polygon: true,
              trash: true
          },
          // styles: polygonStyles,
          userProperties: true
        })
        this.mapbox.addControl(this.currentDraw);

        this.mapbox.on('draw.create', () => { this.readyToCreate = true });
        // this.mapbox.on('draw.delete', this.updateDrawArea);
        this.mapbox.on('draw.update', this.setPolygonStyle);
      } else {
        // add all the stories to the map
        await this.stories.length > 0
        this.stories.forEach(story => {
          this.addToMap(story)
        })
      }

      this.$emit('loaded')
    },
    setPolygonStyle (what) {
      console.log(what)
    },
    onClickedCreate () {
      if (!this.readyToCreate) {
        console.log('not ready, please mark area')
        return
      }
      if (this.currentDraw) {
        console.log(this.currentDraw)
        let data = this.currentDraw.getAll();

        this.saveNewArea(data)
        this.currentDraw.deleteAll()
      }
    },
    onClickedStory (s) {
      this.lastClickedID = s.id
      this.setCurrentStory(s)
    },
    onZoomChanged (evt) {
      // save zoom inside store
      this.setLastLocation({lat: evt.map.transform._center.lat, lng: evt.map.transform._center.lng, zoom: evt.map.style.z})
    },
    onCenterChanged (evt) {
      // save location inside store
      this.setLastLocation({lat: evt.map.transform._center.lat, lng: evt.map.transform._center.lng, zoom: evt.map.style.z})
    },
    getStory (id) {
      return this.stories.find((story) => story.id === id)
    },
    addToMap (story) {

      if (!this.mapbox) return
      let addedIds = []
      this.storiesAdded.forEach((s) => addedIds.push(s.id))

      if (addedIds.indexOf(story.id) !== -1) {
        return
      }

      this.mapbox.addSource(story.id, {
        "type": "geojson",
        "data": {
          "type": "Feature",
          "geometry": {
            "type": "Polygon",
            "coordinates": [[...story.geometry.coordinates.map((coord) => {
              return [coord.lng, coord.lat]
            })]]
          },
          "properties": {
            "title": story.title,
            "marker-symbol": "monument"
          }
        }
      });
      this.storiesAdded.push(story)

      /*
      this.mapbox.addLayer({
        'id': story.id,
        'type': 'fill',
        'source': story.id,
        'layout': {},
        'paint': {
        'fill-color': '#088',
        'fill-opacity': 0.8
        }
      })
      */
    },
    saveNewArea (data) {
      console.log("data to save", data)
      let coordData = []
      data.features[0].geometry.coordinates[0].forEach((coord) => {
        console.log(coord)
        coordData.push({
          lng: coord[0],
          lat: coord[1]
        })
      })
      let entity = {
        geometry: {
          coordinates: coordData
        }
      }
      this.storeCreatingStory(entity)

      this.save(this.getCreatingStory)
      this.$router.push('/explore')
    },
    layerInfo (id) {
      return {
        'id': id,
        'type': 'fill',
        'source': id,
        'layout': {},
        'paint': {
        //'fill-pattern': 'pattern',
        'fill-color': '#B50717', // pallete 5
        'fill-opacity': 0.5,
        'fill-outline-color': 'black'
        }
      }
    },
    getCenterLayer (story) {
      let src = this.mapbox.getSource(story.id)
      if (src) {
        let center = window.turf.centroid(src._data);
        return center.geometry.coordinates
      } else {
        return [0,0]
      }
    },
    ...mapActions('map', [
      'setLastLocation'
    ]),
    ...mapActions('story', [
      'save',
      'storeCreatingStory'
    ]),
    ...mapActions('explore', [
      'startAnimation',
      'stopAnimation',
      'setCurrentStory'
      ]),
      ...mapActions('storage', [
        'fetchDownloadPath'
      ])
  },
  watch: {
    stories () {
      if (!this.mapbox) return
      this.stories.forEach(story => {
        this.addToMap(story)
      })
    },
    getCurrentStory (story) {
      if (story === null || !this.mapbox) {
        return
      }

      var px = this.mapbox.project(this.getCenterLayer(story)); // find the pixel location on the map where the popup anchor is
      px.y -= window.innerHeight/3 // find the height of the popup container, divide by 2, subtract from the Y axis of marker location
      this.mapbox.panTo(this.mapbox.unproject(px),{animate: true}); // pan to new center
      /*
      this.mapbox.flyTo({
        center: [this.getCenterLayer(story)[0], y],
        essential: true,
        zoom: story.zoom ? story.zoom : 4
      })
      */
      setTimeout(() => {
        let marker = document.querySelector('#marker_' + story.id)
        console.log(marker)
        console.log(document.activeElement)
        if (this.lastClickedID !== story.id) {
          marker.click()
        }
      }, 200)
    }
  }
}
</script>

<style lang='sass'>
.mapboxgl-popup-tip
  width: 0
  padding-top: 5px
  height: 100px !important
  border: 1px solid red !important
  z-index: 1

.mapboxgl-popup-content
  background: $white-1
  padding: 0px !important
  height: fit-content

.mapboxgl-popup-close-button
  color: $white-1 !important
  font-size: 144% !important

.marker
  height: 15px !important
  width: 15px
  background-color: red
  border-radius: 50%
  display: inline-block
  cursor: pointer

.slide-fade-enter-active
  transition: all 1s ease !important

.slide-fade-leave-active
  transition: all 1s cubic-bezier(1.0, 0.5, 0.8, 1.0) !important

.slide-fade-enter, .slide-fade-leave-to
  transform: translateX(100px) !important
  opacity: 0 !important

.storycard_body
.storycard_title
  font-size: 16px !important
  text-align: left
  margin-left: 10px !important
  margin-top: 5px !important

.storycard_description
  padding: 10px !important
  text-align: left
</style>


<style lang='sass' scoped>
.calculation-box
  z-index: 1000
  margin-left: 20px
  height: 170px
  width: 150px
  position: absolute
  bottom: 100px
  left: 10px
  background-color: rgba(255, 255, 255, 0.9)
  padding: 15px
  text-align: center

  p
    margin: 0
    font-size: 13px

.createbutton
  z-index: 1001
  position: absolute
  bottom: 115px
  left: 42px
</style>
