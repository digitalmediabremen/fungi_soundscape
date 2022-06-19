module.exports = {
  "css": {
    "loaderOptions": {
      "sass": {
        "prependData": "@import '@/assets/css/style.sass'"
      }
    }
  },
  "transpileDependencies": [
    "vuetify"
  ]
}