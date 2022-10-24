<template>
  <div class="home-container">
    <el-card style="margin-bottom:30px;">
      <el-row :gutter="20" style="margin-bottom:15px;">
        <!-- 顯示狀態,功能 -->
        <el-row :gutter="20" style="margin-bottom:10px;">
          <el-col :span="2">
            <div class="emq-title">
              功能區
            </div>
          </el-col>
          <!-- MQTT連接狀態 -->
          <el-col :span="6">
            <div class="emq-title">
              MQTT連接狀態: {{ client.connected ? "已連接" : "未連接" }}
            </div>
          </el-col>
          <el-col :span="3" v-if="client.connected">
            <el-button :disabled="FunctionMenu.m_SunnyRainy === 'Sunny'" size="small" class="sunny-btn"
              @click="Sunny_RainyClick('Sunny')">
              晴天設定
            </el-button>
          </el-col>
          <el-col :span="3" v-if="client.connected">
            <el-button :disabled="FunctionMenu.m_SunnyRainy === 'Rainy'" size="small" class="rainy-btn"
              @click="Sunny_RainyClick('Rainy')">
              雨天設定
            </el-button>
          </el-col>
        </el-row>
        <el-row :gutter="20" style="margin-bottom:10px;" v-if="client.connected">
          <!-- 查看自己定位 -->
          <el-col :span="8">
            <el-button :disabled="FunctionMenu.m_MyPosition" size="small" class="conn-btn" round
              @click="buttonclick('MyPosition')">
              查看自己定位
            </el-button>
          </el-col>
          <!-- 偵測進入區域功能 -->
          <el-col :span="8">
            <el-button :disabled="FunctionMenu.m_AreaMsg" size="small" class="conn-btn" round
              @click="buttonclick('AreaMsg')">
              偵測進入區域功能(控制區域燈光)
            </el-button>
          </el-col>
          <!-- 顯示物品位置圖 -->
          <el-col :span="8">
            <el-button :disabled="FunctionMenu.m_ViewObject" size="small" class="conn-btn" round
              @click="buttonclick('ViewObject')">
              顯示物品位置圖
            </el-button>
          </el-col>
          <!-- 測試按鈕 -->
          <!-- <el-col :span="8">
            <el-button size="small" class="conn-btn" round @click="DrawPhoto()">
              測試按鈕
            </el-button>
          </el-col> -->
        </el-row>
      </el-row>
      <!-- 目前啟用功能 -->
      <el-row :gutter="20" style="margin-bottom:10px;">
        <el-col :span="8">
          <div>目前啟用功能:
            {{ FunctionMenu.m_AreaMsg ? '偵測進入區域功能' : FunctionMenu.m_ViewObject ? '顯示物品位置圖' : FunctionMenu.m_MyPosition ?
                '查看自己定位' : '目前無啟用功能'
            }}
          </div>
        </el-col>
      </el-row>
      <!-- 偵測到beacon數量 -->
      <el-row :gutter="20" style="margin-bottom:10px;">
        <el-col :span="8">
          <div>偵測到beacon數量: {{ Message.UserMessage.AmountOfBeacon }}</div>
        </el-col>
      </el-row>
      <el-row :gutter="20" style="margin-bottom:10px;">
        <el-col :span="6">
          <div>ESP Station 啟用設定 : {{ FunctionMenu.m_SunnyRainy === 'Sunny' ? ' 晴天功能' : ' 雨天功能' }}</div>
        </el-col>
      </el-row>
    </el-card>
    <!-- 選擇Beacon -->
    <div v-if="FunctionMenu.m_MyPosition || FunctionMenu.m_AreaMsg">
      <el-card style="margin-bottom:30px;">
        <div class="emq-title">
          選擇Beacon
        </div>
        <el-select v-model="Message.FuntionMessage.ChioceDevice">
          <el-option v-for="(item, index) in Message.UserMessage.MajorMinor"
            :key="Message.UserMessage.MajorMinor[index]" :label="Message.UserMessage.MajorMinor[index]"
            :value="Message.UserMessage.MajorMinor[index]">
          </el-option>
        </el-select>
      </el-card>
    </div>
    <!-- Canvas (繪圖)-->
    <div
      v-if="(FunctionMenu.m_ViewObject) || (Message.FuntionMessage.ChioceDevice !== '' && FunctionMenu.m_MyPosition === true)">
      <el-card style="margin-bottom:30px;">
        <canvas id="mycanvas" width="100%" height="100%" class="my-canvas"></canvas>
      </el-card>
    </div>
  </div>
</template>

<script>
import mqtt from 'mqtt'
import trilat from 'trilat' //trilat 解方程式
import axios from 'axios' //axios,qs 串接line
import qs from 'qs'

//引入圖片 indoor,target
import indoor from "@/assets/img/room.png"
//import target from "@/assets/img/target.png"

export default {
  name: 'Home',
  data() {
    return {
      //Main Message
      Message: {
        StationsInfo: [
          //原始數據
          // esp1 x:0 y:30
          // esp2 x:240 y:640
          // esp3 x:940 y:30 
          //設定esp初始數值

          //晴天版
          { name: "esp1", x: 0, y: 20, DistanceMeter: 55, EnvironFator: 3.5, px: 65 },
          { name: "esp2", x: 400, y: 420, DistanceMeter: 55, EnvironFator: 3.5, px: 65 },
          { name: "esp3", x: 940, y: 20, DistanceMeter: 55, EnvironFator: 3.5, px: 65 },
          //雨天版
          // { name: "esp1", x: 0, y: 20, DistanceMeter: 55, EnvironFator: 3, px: 65 },
          // { name: "esp2", x: 500, y: 400, DistanceMeter: 55, EnvironFator: 3, px: 65 },
          // { name: "esp3", x: 940, y: 20, DistanceMeter: 55, EnvironFator: 3, px: 65 },
        ],
        UserMessage: {
          //偵測到的beacon資訊(已處理過) //ex. beacons[m_m][station]
          beacons: [],
          //偵測到的MajorMinor
          MajorMinor: [],
          //計算完位置訊息
          PositionMessage: [],
          AmountOfBeacon: 0,
        },
        FuntionMessage: {
          ChioceDevice: "",
          AreaIndex: 0, //進入房間 1、客廳 2、廁所 3 預設0 判斷是否進入過哪裡
        },
      },
      //Funtion Menu
      FunctionMenu: {
        m_ViewObject: false,
        m_AreaMsg: false,
        m_MyPosition: false,
        m_SunnyRainy: "Sunny",
        m_ViewObject_Run: false,
      },
      linemsg: "",
      //MQTT Msg
      connection: {
        host: '127.0.0.1',
        port: 1884,
        endpoint: '',
        clean: true, // 保留會話
        connectTimeout: 4000, // Timeout
        reconnectPeriod: 4000, // 重新連接間隔
        clientId: 'MqttWeb_indoor',
      },
      subscription: {
        topic: 'indoor/#',
        qos: 0,
      },
      publish: {
        topic: 'indoorLED/esp32',
        qos: 2,
        payload: 'R',
      },
      qosList: [
        { label: 0, value: 0 },
        { label: 1, value: 1 },
        { label: 2, value: 2 },
      ],
      client: {
        connected: false,
      },
    }
  },
  mounted() {
    //創建MQTT連接
    this.createConnection()
  },
  methods: {
    total(message) {
      //處理JSON，RSSI取平均值，波動大不取。
      this.JsonProcess(message)

      let MajorMinor = this.Message.UserMessage.MajorMinor
      let beacons = this.Message.UserMessage.beacons
      let stations = this.Message.StationsInfo

      for (let index = 0; index < MajorMinor.length; index++) {
        if (Object.keys(beacons[MajorMinor[index]]).length >= 3) {      //最少偵測到三台ESP32才計算
          let [x, y] = this.CalculatePosition(beacons[MajorMinor[index]], stations)

          //紀錄位置訊息，下次取平均
          this.Message.UserMessage.PositionMessage[MajorMinor[index]] = { "x": x + 50, "y": y + 50 }
        }
      }

      ////  功能區  ////
      if (Object.keys(this.Message.UserMessage.PositionMessage).length > 0) { //計算完位置的beacon大於0個
        //所有功能開始
        if (this.FunctionMenu.m_AreaMsg) {  //進入偵測功能
          if (this.Message.FuntionMessage.ChioceDevice == "") {
            console.log("尚未選取設備")
          } else {
            this.AreaMsg()
          }
        } else if (this.FunctionMenu.m_MyPosition) {
          if (this.Message.FuntionMessage.ChioceDevice == "") {
            console.log("尚未選取設備")
          } else {
            this.MyPosition()
          }
        } else if (this.FunctionMenu.m_ViewObject) {   //查看定位物品圖功能
          if (!this.FunctionMenu.m_ViewObject_Run) {
            this.ViewObject()
            this.FunctionMenu.m_ViewObject_Run = true
          }
        }
      }

    },
    ////功能區////
    AreaMsg() {
      console.log("使用AreaMsg功能")

      //取得選取的beacon
      let SelectDevice = this.Message.FuntionMessage.ChioceDevice
      //取得鎖定beacon的x,y
      let [x, y] = [this.Message.UserMessage.PositionMessage[SelectDevice].x, this.Message.UserMessage.PositionMessage[SelectDevice].y]

      //定位區域的位置
      //Start [x,y] End [x,y]
      let room_x_start = 0, room_y_start = 0, room_x_end = 460, room_y_end = 270 //A
      let living_x_start = 590, living_y_start = 0, living_x_end = 1000, living_y_end = 440 //B
      let balcony_x_start = 170, balcony_y_start = 290, balcony_x_end = 580, balcony_y_end = 450  //C

      // A:房間
      // B:客廳
      // C:陽台

      //進行位置判斷並發送Line訊息

      // A:房間
      if (x > room_x_start && x < room_x_end && y > room_y_start && y < room_y_end) {
        if (this.Message.FuntionMessage.AreaIndex != 1) {
          this.linemsg = "進入房間"
          this.SendLineMsg()
          this.Message.FuntionMessage.AreaIndex = 1
        }
      }
      // B:客廳
      if (x > living_x_start && x < living_x_end && y > living_y_start && y < living_y_end) {
        if (this.Message.FuntionMessage.AreaIndex != 2) {
          this.linemsg = "進入客廳"
          this.SendLineMsg()
          this.Message.FuntionMessage.AreaIndex = 2
        }
      }
      // C:陽台
      if (x > balcony_x_start && x < balcony_x_end && y > balcony_y_start && y < balcony_y_end) {
        if (this.Message.FuntionMessage.AreaIndex != 3) {
          this.linemsg = "進入廁所"
          this.SendLineMsg()
          this.Message.FuntionMessage.AreaIndex = 3
        }
      }
      switch (this.Message.FuntionMessage.AreaIndex) {
        case 1:
          this.doPublish("A")
          break;
        case 2:
          this.doPublish("B")
          break;
        case 3:
          this.doPublish("C")
          break;
        default:
          this.doPublish("R")
          break;
      }


    },
    MyPosition() {
      console.log("MyPosition功能")
      this.DrawMyPosition()
    },
    ViewObject() {
      console.log("ViewObject功能")
      this.DrawViewObject()
    },
    SendLineMsg() {
      let msg = this.linemsg
      axios({
        method: "post",
        url: "/api/linemsg/with/key/c5e3V-0szcmJDeImLlXzai",
        transformRequest: [function (data) {
          return qs.stringify(data)
        }],
        data: {
          value1: msg
        }
      }).catch((error) => console.log(error))
      console.log("已發送: " + msg)
    },
    //功能按鈕
    buttonclick(str) {
      switch (str) {
        case "AreaMsg":
          this.FunctionMenu.m_AreaMsg = true
          this.FunctionMenu.m_MyPosition = false
          this.FunctionMenu.m_ViewObject = false

          this.FunctionMenu.m_ViewObject_Run = false  //查看物品定位圖重設
          break
        case "MyPosition":
          this.FunctionMenu.m_AreaMsg = false
          this.FunctionMenu.m_MyPosition = true
          this.FunctionMenu.m_ViewObject = false

          this.FunctionMenu.m_ViewObject_Run = false  //查看物品定位圖重設

          this.Message.FuntionMessage.AreaIndex = 0 //判斷進入的功能重設
          break
        case "ViewObject":
          this.FunctionMenu.m_AreaMsg = false
          this.FunctionMenu.m_MyPosition = false
          this.FunctionMenu.m_ViewObject = true

          this.Message.FuntionMessage.AreaIndex = 0 //判斷進入的功能重設
          break
      }
    },
    Sunny_RainyClick(str) {
      switch (str) {
        case "Sunny":
          this.Message.StationsInfo =
            [{ name: "esp1", x: 0, y: 20, DistanceMeter: 55, EnvironFator: 3.5, px: 65 },
            { name: "esp2", x: 400, y: 420, DistanceMeter: 55, EnvironFator: 3.5, px: 65 },
            { name: "esp3", x: 940, y: 20, DistanceMeter: 55, EnvironFator: 3.5, px: 65 },]
          this.FunctionMenu.m_SunnyRainy = "Sunny"
          console.log("Esp Station Setting: Sunny Model")
          break
        case "Rainy":
          this.Message.StationsInfo =
            [{ name: "esp1", x: 0, y: 20, DistanceMeter: 55, EnvironFator: 3, px: 65 },
            { name: "esp2", x: 400, y: 420, DistanceMeter: 55, EnvironFator: 3, px: 65 },
            { name: "esp3", x: 940, y: 20, DistanceMeter: 55, EnvironFator: 3, px: 65 },]
          this.FunctionMenu.m_SunnyRainy = "Rainy"
          console.log("Esp Station Setting: Rainy Model")
          break
      }
    },

    JsonProcess(message) {

      let msg = JSON.parse(message)

      // !== 相同型別才做比較
      //參考https://github.com/simonbogh/ESP32-iBeacon-indoor-positioning

      //RSSI取平均且波動太大不取

      if (msg !== null) {
        for (let i = 0; i < msg.info.length; i++) {
          //接收資料
          let station = msg.station
          let m_m = msg.info[i].major + "," + msg.info[i].minor
          if (m_m == "0,0") return  //去除預設 0,0之設備
          let ReceiveRssi = msg.info[i].rssi
          //網站資料
          let beacons = this.Message.UserMessage.beacons
          let MajorMinor = this.Message.UserMessage.MajorMinor

          //判斷beacons[m_m]是否存在
          if (typeof (beacons[m_m]) !== 'object') {
            // Initialize
            beacons[m_m] = []
            MajorMinor.push(m_m)

            this.Message.UserMessage.AmountOfBeacon += 1 //beacon數量增加
          }
          //存入or更新Beacon RSSI
          if (beacons[m_m][station] == null) {
            beacons[m_m][station] = parseInt(ReceiveRssi, 10) //New

          }
          else {
            beacons[m_m][station] = parseInt((ReceiveRssi + beacons[m_m][station]) / 2, 10) //Update
          }
        }
      }

    },
    //CalculatePosition return [x, y]
    CalculatePosition(beacon, stations) {
      function CalculateDistance(rssi, station) {
        // Distance = 10^((abs(RSSI) - DistanceMeter) / (10 * EnvironFator))
        // RSSI - 接收訊號強度(負值)
        // DistanceMeter - 發收跟接收相距1米的訊號強度
        // EnvironFator - 環境衰減因子
        let DistanceMeter = station.DistanceMeter
        let EnvironFator = station.EnvironFator
        let Px = station.px
        let Distance = Math.pow(10, (Math.abs(rssi) - DistanceMeter) / (10 * EnvironFator))

        return Distance * Px;
      }

      let input = [
        [stations[0].x, stations[0].y, CalculateDistance(beacon.esp1, stations[0])],
        [stations[1].x, stations[1].y, CalculateDistance(beacon.esp2, stations[1])],
        [stations[2].x, stations[2].y, CalculateDistance(beacon.esp3, stations[2])]
      ]

      //console.log(input)
      let output = trilat(input)
      return [output[0], output[1]]
    },

    /////畫圖功能/////
    DrawViewObject() {
      let canvas = document.getElementById("mycanvas");//New canvas
      let context = canvas.getContext("2d"); //創立畫布

      this.DrawPhoto(context)
    },
    DrawMyPosition() {
      let canvas = document.getElementById("mycanvas");//New canvas
      let context = canvas.getContext("2d"); //創立畫布

      this.DrawPhoto(context)
    },

    DrawStations(context) {
      let m_Stations = this.Message.StationsInfo

      for (let index = 0; index < m_Stations.length; index++) {
        context.beginPath();
        context.arc(m_Stations[index].x + 20, m_Stations[index].y + 20, 12, 0, 2 * Math.PI);
        context.fillStyle = "#636766"; //設定填滿圖形時用的顏色.
        context.fill();

        context.font = "20px Arial bolder"
        context.fillStyle = 'red'
        context.fillText(m_Stations[index].name, m_Stations[index].x + 15, m_Stations[index].y + 50)
      }
    },
    DrawTarget(context, PositionMessage) {
      let MajorMinor = this.Message.UserMessage.MajorMinor
      let FunctionMenu = this.FunctionMenu
      //判斷選取功能 自己位置or物品位置圖
      if (FunctionMenu.m_MyPosition) {  //自己位置功能
        let SelectDevice = this.Message.FuntionMessage.ChioceDevice

        context.beginPath();
        context.arc(PositionMessage[SelectDevice].x + 20, PositionMessage[SelectDevice].y + 20, 12, 0, 2 * Math.PI);
        context.fill();

        context.font = "20px Arial bolder"
        context.fillStyle = 'red'
        context.fillText(SelectDevice, PositionMessage[SelectDevice].x + 15, PositionMessage[SelectDevice].y + 50)
      }
      else if (FunctionMenu.m_ViewObject) { //物品位置圖
        for (let index = 0; index < MajorMinor.length; index++) {
          if (typeof (PositionMessage[MajorMinor[index]]) !== "undefined") {
            context.beginPath();
            context.arc(PositionMessage[MajorMinor[index]].x + 20, PositionMessage[MajorMinor[index]].y + 20, 12, 0, 2 * Math.PI);
            context.fill();

            context.font = "20px Arial bolder"
            context.fillStyle = 'red'
            context.fillText(MajorMinor[index], PositionMessage[MajorMinor[index]].x + 15, PositionMessage[MajorMinor[index]].y + 50)
          }
        }
      }


    },
    DrawPhoto() {
      let canvas = document.getElementById("mycanvas");//New canvas
      let context = canvas.getContext("2d"); //創立畫布

      //let TargetImg = new Image(); //New Object
      let IndoorImg = new Image();

      //TargetImg.src = target;
      IndoorImg.src = indoor;

      //load image
      IndoorImg.onload = () => {
        canvas.setAttribute("width", IndoorImg.width)
        canvas.setAttribute("height", IndoorImg.height)

        //繪製底圖
        context.drawImage(IndoorImg, 0, 0, IndoorImg.width, IndoorImg.height)

        //繪製基地台
        this.DrawStations(context)
        //繪製目標
        this.DrawTarget(context, this.Message.UserMessage.PositionMessage)
      }

    },
    //測試畫圖功能
    DrawTablePhoto(context) {
      let x = 1100
      let y = 600
      let BlockSize = 20
      //畫座標 每格100
      for (let i = BlockSize; i <= x; i += 100) {
        context.fillText(i - BlockSize, i, BlockSize - 10)
      }
      for (let i = BlockSize; i <= y; i += 100) {
        context.fillText(i - BlockSize, 0, i)
      }
      //畫格子
      context.strokeStyle = 'black'
      context.lineWidth = 1
      //畫格子X軸
      for (let i = BlockSize; i <= x; i += BlockSize) {
        context.moveTo(i, BlockSize)
        context.lineTo(i, y)
        context.stroke()
      }
      //畫格子Y軸
      for (let i = BlockSize; i <= y; i += BlockSize) {
        context.moveTo(BlockSize, i)
        context.lineTo(x, i)
        context.stroke()
      }

    },
    DrawTargetTest(context) {
      //1
      context.beginPath();
      context.arc(200 + 20, 100 + 20, 12, 0, 2 * Math.PI);
      context.fill();

      context.font = "20px Arial bolder"
      context.fillStyle = 'red'
      context.fillText("1,1", 200 + 15, 100 + 40)
      //2
      context.beginPath();
      context.arc(700 + 20, 50 + 20, 12, 0, 2 * Math.PI);
      context.fill();

      context.font = "20px Arial bolder"
      context.fillStyle = 'red'
      context.fillText("1,2", 700 + 15, 50 + 40)
      //3
      context.beginPath();
      context.arc(400 + 20, 150 + 20, 12, 0, 2 * Math.PI);
      context.fill();

      context.font = "20px Arial bolder"
      context.fillStyle = 'red'
      context.fillText("1,3", 400 + 15, 150 + 40)
      //4
      context.beginPath();
      context.arc(150 + 20, 300 + 20, 12, 0, 2 * Math.PI);
      context.fill();

      context.font = "20px Arial bolder"
      context.fillStyle = 'red'
      context.fillText("1,4", 150 + 15, 300 + 40)
    },

    //MQTT Connection,subscribe...
    createConnection() {
      // ws 未加密 WebSocket 連接
      // wss 加密 WebSocket 連接
      // mqtt 未加密 TCP 連接
      // mqtts 加密 TCP 連接
      const { host, port, endpoint, ...options } = this.connection
      const connectUrl = `ws://${host}:${port}${endpoint}`

      try {
        this.client = mqtt.connect(connectUrl, options)
        //Subscribe
        this.doSubscribe()
      } catch (error) {
        console.log('mqtt.connect error', error)
      }
      this.client.on('connect', () => {
        console.log('Connection succeeded!')
      })
      this.client.on('error', error => {
        console.log('Connection failed', error)
      })
      //進入點
      this.client.on('message', (topic, message) => {
        this.total(message)
      })
    },
    doSubscribe() {
      const { topic, qos } = this.subscription
      this.client.subscribe(topic, { qos }, (error, res) => {
        if (error) {
          console.log('Subscribe to topics error', error)
          return
        }
        console.log('Subscribe to topics res', res)
      })
    },
    doPublish(msg) {
      console.log("Publish LED Area")
      const { topic, qos, payload } = this.publish
      //qos使用2層級 
      this.publish.payload = msg //修改傳出msg

      this.client.publish(topic, payload, qos, error => {
        if (error) {
          console.log('Publish error', error)
        }
      })
    },
    destroyConnection() {
      if (this.client.connected) {
        try {
          this.client.end()
          this.client = {
            connected: false,
          }
          console.log('Successfully disconnected!')
        } catch (error) {
          console.log('Disconnect failed', error.toString())
        }
      }
    },
  },
}
</script>


<style lang="scss">
@import url('../assets/style/home.scss');

.home-container {
  max-width: 1100px;
  margin: 0 auto;

  .conn-btn {
    color: #fff;
    background-color: #00b173;
    font-size: 14px;
  }

  .sunny-btn {
    color: rgb(255, 255, 255);
    background-color: rgb(255, 153, 0);
    font-size: 14px;
  }

  .rainy-btn {
    color: rgb(255, 255, 255);
    background-color: rgb(0, 102, 255);
    font-size: 14px;
  }

  .publish-btn {
    margin-bottom: 20px;
    float: right;
  }

  .el-button--success {
    background-color: #34c388 !important;
    border-color: #34c388 !important;
    font-size: 14px !important;
  }

  .el-button--success:hover {
    background-color: #f5222d !important;
    border-color: #f5222d !important;
    font-size: 14px !important;
  }

  .el-button--danger {
    background-color: #f5222d !important;
    border-color: #f5222d !important;
  }

  .el-form-item {
    &.is-error {

      .el-input__inner,
      .el-textarea__inner {
        box-shadow: 0 0 0 2px rgba(245, 34, 45, 0.2);
      }
    }

    &.is-success {

      .el-input__inner,
      .el-textarea__inner {
        border-color: #34c388 !important;
      }
    }
  }
}
</style>
