const state = {
  modal: {
    isOpen: false,
    args: Object,
    oldArgs: null,
    firstModalType: ''
  }
}

const actions = {
  openModal ({commit, getters}, args) {
    if (args.type === getters.getModalType) {
      commit('set_modal_closed')
      setTimeout(() => {
        commit('set_modal_open', args)
      }, 30)
    } else {
      commit('set_modal_open', args)
    }
  },
  closeModal ({commit}) {
    commit('set_modal_closed')
  }
}

const mutations = {
  set_modal_open (state, args) {
    if (state.modal.oldArgs === null) {
      state.modal.firstModalType = args.type
    }
    state.modal.oldArgs = Object.assign({}, state.modal.args)
    state.modal.isOpen = true
    state.modal.args = args
  },
  set_modal_closed (state) {
    state.modal.isOpen = false
    state.modal.args = null
    state.modal.oldArgs = null
  }
}

const getters = {
  isModalOpen (state) {
    return state.modal.isOpen
  },
  getModalType (state) {
    let modalType = ''
    if (state.modal.args !== null) {
      modalType = state.modal.args.type
    }
    return modalType
  },
  getModalArgs (state) {
    return state.modal.args
  },
  getModalOldArgs (state) {
    return state.modal.oldArgs
  },
  getFirstOpenedModal (state) {
    return state.modal.firstModalType
  }
}

export default {
  state,
  actions,
  mutations,
  getters
}
