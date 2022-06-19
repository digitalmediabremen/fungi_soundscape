
/*
const fetch = url =>
  services(url)
    .get()
    .then(response => response.data)
*/

// fake API

const fetch = () =>
  new Promise((resolve) =>
    resolve({
      status: 200,
      detail: {
        'stories': [
          {
            id: 0,
            title: 'Story 1',
            description: 'Lorem ipsum dolor sit amet.',
            date: 1545096864,
            thumbnail: '',
            author: 'Samantha',
            lat: -22.913731,
            lng: -43.182279,
            geometry: {
              coordinates: [
                [102.11806158995955, 69.08880731687233],
                [100.02547091545694, 66.13248462554594],
                [109.89054123810831, 67.14001090921067],
                [102.11806158995955, 69.08880731687233]
              ]
            },
            photos: [
              {
                url: ''
              },
              {
                url: ''
              },
              {
                url: ''
              }
            ]
          }
        ]
      }
    }))

export default fetch
