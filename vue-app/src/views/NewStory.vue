<template>
  <v-col class='newstory overflow-y-auto' align-content="center">
    <h2 class='newstory_title mb-8'>Report incident / crime</h2>
    <v-row class='newstory_body' align="center" justify="center" align-content="center">
      <v-form
        class='newstory_body_form mx-4'
        ref="form"
        v-model="valid"
        :lazy-validation="lazy"
      >
        <!--
        <v-text-field
          v-model="name"
          :counter="10"
          :rules="nameRules"
          label="Name"
          required
        ></v-text-field>
        -->

        <v-text-field
          v-model="title"
          :rules="titleRules"
          label="Title"
          required
        ></v-text-field>


        <v-textarea
          v-model="description"
          name="input-7-1"
          filled
          label="Description"
          auto-grow
          full-width
          value=""
          :rules="descriptionRules"
        ></v-textarea>


        <v-text-field
          v-model="city"
          :rules="cityRules"
          label="City"
          required
        ></v-text-field>

        <v-text-field
          v-model="country"
          :rules="countryRules"
          label="Country"
          required
        ></v-text-field>

        <v-select
          class='mt-4'
          v-model="crimeType"
          :items="types"
          :rules="[v => !!v || 'Type is required']"
          label="Type"
          required
        ></v-select>

        <v-text-field
          v-if="crimeType === 'Other'"
          v-model="specificCrimeType"
          :rules="[v => !!v || 'Specify type is required']"
          label="Please specify"
          required
        ></v-text-field>

        <v-btn
          :loading="loading"
          tile
          :disabled="!valid"
          color="white"
          class="mt-4"
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
  mounted () {
    if (this.getCreatingStory) {
      this.title = this.getCreatingStory.title
      this.description = this.getCreatingStory.description
      this.crimeType = this.getCreatingStory.type
      this.city = this.getCreatingStory.city
      this.country = this.getCreatingStory.country
    }
  },
  computed: {
    ...mapGetters('auth', [
      'getUser'
    ]),
    ...mapGetters('story', [
      'getCreatingStory'
    ])
  },
  data: () => ({
    valid: true,
    title: '',
    titleRules: [
      v => !!v || 'Title is required'
    ],
    description: '',
    descriptionRules: [
      v => !!v || 'Description is required'
    ],
    city: '',
    cityRules: [
      v => !!v || 'City is required'
    ],
    country: '',
    countryRules: [
      v => !!v || 'Country is required'
    ],
    crimeType: '',
    types: [
      'Illegal logging',
      'Envinronmental Killings',
      'Poaching/Hunting',
      'Abandoned vehicle',
      'Bushmeat',
      'Wildlife smuggling',
      'Illegal construction',
      'Illegal dumping',
      'Illegal mining',
      'Illegal fishing',
      'Littering',
      'Dumping',
      'Irregular toxic waste disposal',
      'Irregular e-waste disposal',
      'Oil spill',
      'Water pollution'
    ],
    lazy: false,
    loading: false,
    error: '',
    specificCrimeType: ''
  }),

  methods: {
    ...mapActions('auth', [
      'login'
    ]),
    ...mapActions('story', [
      'storeCreatingStory'
    ]),
    submit () {
      let entity = {
        title: this.title,
        description: this.description,
        date: 1545096864,
        type: this.crimeType === 'Other' ? this.crimeType : this.specificCrimeType,
        thumbnail: '',
        zoom: 0,
        authorName: this.getUser.displayName,
        authorID: this.getUser.uid,
        city: this.city,
        country: this.country,
        geometry: {
          coordinates: []
        },
        images: [
        ]
      }

      this.storeCreatingStory(entity)
      this.$router.push('/create/2')
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

<style lang='sass' scoped>
.newstory
  max-height: 80vh

  &_title
    margin-top: 40px
  &_body
    &_form
      width: 100%
      max-width: 400px
</style>