<template>
  <v-col class='newstoryfiles overflow-y-auto' align-self="center" >
    <h2 class='newstoryfiles_title mb-8'>Add incident</h2>
    <v-row class='newstoryfiles_body' align="center" justify="center" align-content="center">
      <v-form
        class='newstoryfiles_body_form mx-4'
        ref="form"
        v-model="valid"
        :lazy-validation="lazy"
      >

        <v-file-input accept="image/*" :loading="getShowProgress" full-width chips multiple label="Upload images of the incident" @change="onFilePick" @click:clear="onClear">
          <template v-slot:progress>
            <v-progress-linear

              v-model="getUploadProgress"
            ></v-progress-linear>
          </template>
        </v-file-input>


        <v-btn
          tile
          color="white"
          class="mt-4 mx-4"
          @click="$router.go(-1)"

        >
          Previous
        </v-btn>
        <v-btn
          :loading="getShowProgress"
          tile
          :disabled="!finished"
          color="white"
          class="mt-4 mx-4"
          @click="submit"

        >
          Next
        </v-btn>
        <v-alert class='mt-6' v-if="error !== ''" type="error" tile>
        {{ error }}
        </v-alert>

      </v-form>
    </v-row>
  </v-col>
</template>
<script>
import { mapActions, mapGetters } from 'vuex'

export default {
  name: 'newstory',
  components: {
  },
  computed: {
    ...mapGetters('auth', [
      'getUser'
    ]),
    ...mapGetters('storage', [
      'getUploadProgress'
    ]),
    getShowProgress () {
      console.log(this.getUploadProgress)
      return this.getUploadProgress > 1 && this.getUploadProgress < 100 && !this.finished
    }
  },
  data: () => ({
    valid: true,
    lazy: false,
    error: '',
    finished: false,
    files: []
  }),
  methods: {
    ...mapActions('auth', [
      'login'
    ]),
    ...mapActions('story', [
      'storeCreatingStory'
    ]),
    ...mapActions('storage', [
      'uploadFile'
    ]),
    onClear () {
      this.finished = false
    },
    onFilePick (files) {

      if (!files || files.length === 0) {
        this.finished = false
        return
      }
      this.finished = false
      this.files = []
      let timestamp = Date.now()
      files.forEach((file) => {
        let url = this.getUser.uid + '/images/' + timestamp + '/' + file.name
        this.uploadFile({
          file: file,
          url: url
        }).then((downloadUrl) => {
          console.log('uploaded!' + downloadUrl)
          console.log(file)
          let dataToSave = {
            name: file.name,
            url: url,
            downloadUrl: downloadUrl
          }
          this.files.push(dataToSave)
          this.finished = true
        })
      })
    },
    submit () {
      let entity = {
        images: this.files,
        thumbnail: this.files[0]
      }
      this.storeCreatingStory(entity)

      this.$router.push('/create/3')
    },
    reset () {
      this.$refs.form.reset()
    },
    resetValidation () {
      this.$refs.form.resetValidation()
    },
  }
}
</script>
<style lang='sass'>

</style>
<style lang='sass' scoped>
.newstoryfiles
  max-height: 80vh

  &_title
    margin-top: 40px
  &_body
    &_form
      width: 100%
      max-width: 400px
</style>