<template>
  <v-col align-self="center" >
    <v-row class='login fill-height' align="center" justify="center" align-content="center">
      <v-form
        class='login_form'
        ref="form"
        v-model="valid"
        :lazy-validation="lazy"
      >
        <v-text-field
          v-model="name"
          :counter="10"
          :rules="nameRules"
          label="Name"
          required
        ></v-text-field>

        <v-text-field
          class='mt-4'
          v-model="email"
          :rules="emailRules"
          label="E-mail"
          required
        ></v-text-field>

        <v-text-field
          class='mt-4'
          required
          autocomplete="current-password"
          label="Password"
          :append-icon="showPassword ? 'mdi-eye' : 'mdi-eye-off'"
          @click:append="() => (showPassword = !showPassword)"
          :type="password && !showPassword ? 'password' : 'text'"
          :rules="passwordRegisterRules"
          v-model="password"
        ></v-text-field>

        <v-select
          class='mt-4'
          v-model="select"
          :items="items"
          :rules="[v => !!v || 'Item is required']"
          label="Profession"
          required
        ></v-select>

        <v-checkbox
          class='mt-4'
          v-model="checkbox"
          :rules="[v => !!v || 'You must agree to continue!']"
          label="Do you agree with the terms of service?"
          required
        ></v-checkbox>

        <v-btn
          :loading="loading"
          tile
          :disabled="!valid"
          color="white"
          class="mt-4"
          @click="submit"

        >
          Register
        </v-btn>
        <v-alert class='mt-6' v-if="error !== ''" type="error" tile>
        {{ error }}
        </v-alert>

      </v-form>
        <v-btn
          absolute
          bottom
          text
          color="black"
          class="mt-8"
          @click="$router.push('/login')"

        >
          Or LOGIN
        </v-btn>
    </v-row>
  </v-col>
</template>
<script>
import { mapActions } from 'vuex'

export default {
  name: 'register',
  components: {
  },
  computed: {
  },
  data: () => ({
    valid: true,
    name: '',
    nameRules: [
      v => !!v || 'Name is required',
      v => (v && v.length <= 10) || 'Name must be less than 10 characters',
    ],
    email: '',
    emailRules: [
      v => !!v || 'E-mail is required',
      v => /.+@.+\..+/.test(v) || 'E-mail must be valid',
    ],
    password: '',
    passwordLoginRules: [
      v => !!v || 'Password is required'
    ],
    passwordRegisterRules: [
      v => !!v || 'Password is required',
      v => (v && v.length >= 8) || 'Password must be at least 8 characters',
    ],
    showPassword: false,
    select: null,
    items: [
      'Educator',
      'Environmentalist',
      'Student',
      'Jouralist',
    ],
    checkbox: false,
    lazy: false,
    loading: false,
    error: ''
  }),
  methods: {
    ...mapActions('auth', [
      'register'
    ]),
    submit () {
      this.$refs.form.validate()
      if (this.valid && !this.loading) {
        this.loading = true
        console.log('register')
        this.register({ email: this.email, password: this.password, name: this.name}).then(() => {
          if (this.$route.query.creatingEntry) {
            this.$router.push('/create/1')
          } else {
            this.$router.push('/')
          }
        }).catch((e) => {
          this.loading = false
          this.$refs.form.reset()
          this.$refs.form.resetValidation()

          console.error(e)
          this.error = e.message
          setTimeout(() => {
            this.error = ''
          }, 5000)
        })
      }
    }
  }
}
</script>

<style lang='sass' scoped>
.login_form
  width: 400px
</style>