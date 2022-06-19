<template>
  <v-col align-self="center" >
    <v-row class='login fill-height' align="center" justify="center" align-content="center">
      <v-form
        class='login_form'
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
          :rules="passwordLoginRules"
          v-model="password"
        ></v-text-field>

        <v-btn
          :loading="loading"
          tile
          :disabled="!valid"
          color="white"
          class="mt-4"
          @click="submit"

        >
          Login
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
        @click="$router.push({ path: '/register', query: { creatingEntry: true }})"

      >
        Or Register
      </v-btn>
    </v-row>
  </v-col>
</template>
<script>
import { mapActions } from 'vuex'

export default {
  name: 'login',
  components: {
  },
  computed: {
  },
  mounted () {
    console.log(this.$route.query.creatingEntry)
  },
  data: () => ({
    valid: true,
    email: '',
    emailRules: [
      v => !!v || 'E-mail is required',
      v => /.+@.+\..+/.test(v) || 'E-mail must be valid',
    ],
    password: '',
    passwordLoginRules: [
      v => !!v || 'Password is required'
    ],
    showPassword: false,
    lazy: false,
    loading: false,
    error: ''
  }),

  methods: {
    ...mapActions('auth', [
      'login'
    ]),
    submit () {
      console.log('login')
      this.$refs.form.validate()
      if (this.valid) {
        this.loading = true
        this.login({ email: this.email, password: this.password}).then(() => {
          this.loading = false
          if (this.$route.query.creatingEntry) {
            this.$router.push('/create/1')
          } else {
            this.$router.push('/')
          }
        }).catch((e) => {
          console.error(e)
          this.$refs.form.reset()
          this.$refs.form.resetValidation()

          this.loading = false
          this.error = e.message

          setTimeout(() => {
            this.error = ''
          }, 5000)
        })
        console.log('login')
      }
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
.login_form
  width: 400px
</style>