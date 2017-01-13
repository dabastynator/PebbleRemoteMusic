module.exports = [
  {
    "type": "heading",
    "defaultValue": "MusicControl Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Configure the remote muxic control"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Connection"
      },
      {
        "type": "input",
        "messageKey": "host",
        "defaultValue": "http://asterix:5061/mediaserver",
        "label": "End point"
      },
      {
        "type": "input",
        "messageKey": "id",
        "defaultValue": "media.living",
        "label": "Mediaserver id"
      },
      {
        "type": "input",
        "messageKey": "player",
        "defaultValue": "mplayer",
        "label": "Player"
      },
      {
        "type": "input",
        "messageKey": "token",
        "defaultValue": "",
        "label": "Security token"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];