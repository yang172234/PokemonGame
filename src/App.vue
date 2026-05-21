<template>
  <div class="pokemon-battle">
    <!-- 背景装饰 -->
    <div class="bg-decoration"></div>
    
    <!-- 选择精灵界面 -->
    <div v-if="!gameStarted" class="selection-screen">
      <div class="selection-card">
        <div class="selection-header">
          <img src="/images/pokeball.png" class="pokeball-icon" alt="精灵球" @error="handleImageError">
          <h1>宝可梦对战系统</h1>
        </div>
        
        <div class="pokemon-grid">
          <div 
            v-for="p in pokemonList" 
            :key="p.id"
            class="pokemon-card"
            :class="{ selected: selectedPid === p.id }"
            @click="selectedPid = p.id"
          >
            <div class="pokemon-image-wrapper">
              <img 
                :src="getPokemonImage(p.id)" 
                :alt="p.name"
                class="pokemon-selection-img"
                @error="handleImageError"
              >
            </div>
            <div class="pokemon-name">{{ p.name }}</div>
            <div class="pokemon-id">#{{ p.id.toString().padStart(3, '0') }}</div>
          </div>
        </div>
        
        <button class="start-btn" @click="initGame">
          <span class="btn-text">开始对战</span>
          <span class="btn-icon">⚔️</span>
        </button>
      </div>
    </div>

    <!-- 对战界面 -->
    <div v-else class="battle-screen">
      <!-- 上半部分：对战双方 -->
      <div class="battle-area">
        <!-- 敌方精灵 -->
        <div class="pokemon-side enemy-side">
          <div class="pokemon-info">
            <div class="pokemon-name-tag">
              <span class="pokemon-name">{{ enemy.name }}</span>
              <span class="pokemon-level">Lv.50</span>
            </div>
            
            <div class="hp-container">
              <div class="hp-label">
                <span>❤️ HP</span>
                <span class="hp-value">{{ enemy.currentHp }} / {{ enemy.maxHp }}</span>
              </div>
              <div class="hp-bar-bg">
                <div 
                  class="hp-bar-fill enemy-hp" 
                  :style="{ width: enemyHpPercent + '%' }"
                >
                  <div class="hp-bar-glow"></div>
                </div>
              </div>
            </div>
          </div>
          
          <div class="pokemon-sprite">
            <img 
              :src="getPokemonImage(getPokemonIdByName(enemy.name))" 
              :alt="enemy.name"
              class="battle-sprite enemy-sprite"
              :class="{ 'low-hp': enemyHpPercent < 30 }"
              @error="handleImageError"
            >
            <div class="sprite-effects" v-if="isAttacking === 'enemy'">
              <div class="damage-effect">💥</div>
            </div>
          </div>
        </div>

        <!-- VS 标志 -->
        <div class="vs-divider">
          <div class="vs-circle">
            <span>VS</span>
          </div>
        </div>

        <!-- 玩家精灵 -->
        <div class="pokemon-side player-side">
          <div class="pokemon-sprite">
            <img 
              :src="getPokemonImage(selectedPid)" 
              :alt="player.name"
              class="battle-sprite player-sprite"
              :class="{ 'low-hp': playerHpPercent < 30 }"
              @error="handleImageError"
            >
            <div class="sprite-effects" v-if="isAttacking === 'player'">
              <div class="attack-effect">⚡</div>
            </div>
          </div>
          
          <div class="pokemon-info">
            <div class="pokemon-name-tag">
              <span class="pokemon-name">{{ player.name }}</span>
              <span class="pokemon-level">Lv.50</span>
            </div>
            
            <div class="hp-container">
              <div class="hp-label">
                <span>❤️ HP</span>
                <span class="hp-value">{{ player.currentHp }} / {{ player.maxHp }}</span>
              </div>
              <div class="hp-bar-bg">
                <div 
                  class="hp-bar-fill player-hp" 
                  :style="{ width: playerHpPercent + '%' }"
                >
                  <div class="hp-bar-glow"></div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- 中间操作区 -->
      <div class="action-area" v-if="!gameOver">
        <div class="skills-panel">
          <div class="skills-header">
            <span>✨ 选择招式</span>
            <span class="turn-indicator">你的回合</span>
          </div>
          <div class="skills-grid">
            <button 
              v-for="(skill, idx) in skills" 
              :key="idx"
              class="skill-btn"
              @click="useSkill(idx+1)"
            >
              <div class="skill-name">{{ parseSkillName(skill) }}</div>
              <div class="skill-detail">{{ parseSkillDetail(skill) }}</div>
            </button>
          </div>
        </div>
      </div>

      <!-- 战斗结束界面 -->
      <div v-if="gameOver" class="game-over-panel">
        <div class="result-card">
          <div class="result-icon">{{ gameResult.includes('胜利') ? '🏆' : '💔' }}</div>
          <h2>{{ gameResult }}</h2>
          <button class="restart-btn" @click="resetGame">
            <span>🔄</span> 重新开始
          </button>
        </div>
      </div>

      <!-- 下半部分：战斗日志 -->
      <div class="battle-log">
        <div class="log-header">
          <span>📜 战斗日志</span>
          <button class="clear-log-btn" @click="clearLog">清除</button>
        </div>
        <div class="log-content" ref="logContainer">
          <div 
            v-for="(msg, idx) in battleLog" 
            :key="idx"
            class="log-entry"
            :class="getLogClass(msg)"
          >
            <span class="log-time">{{ formatTime(idx) }}</span>
            <span class="log-message">{{ msg }}</span>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import axios from 'axios'

export default {
  name: 'PokemonBattle',
  data() {
    return {
      selectedPid: 1,
      gameStarted: false,
      player: { name: '', maxHp: 0, currentHp: 0 },
      enemy: { name: '', maxHp: 0, currentHp: 0 },
      skills: [],
      battleLog: [],
      gameOver: false,
      gameResult: '',
      isAttacking: null, // 'player' 或 'enemy'
      pokemonList: [
        { id: 1, name: '皮卡丘' },
        { id: 2, name: '喷火龙' },
        { id: 3, name: '水箭龟' },
        { id: 4, name: '妙蛙花' },
        { id: 5, name: '超梦' },
        { id: 6, name: '梦幻' },
        { id: 7, name: '卡比兽' },
        { id: 8, name: '快龙' },
        { id: 9, name: '伊布' }
      ]
    }
  },
  computed: {
    playerHpPercent() {
      return (this.player.currentHp / this.player.maxHp) * 100
    },
    enemyHpPercent() {
      return (this.enemy.currentHp / this.enemy.maxHp) * 100
    }
  },
  watch: {
    battleLog: {
      handler() {
        this.$nextTick(() => {
          if (this.$refs.logContainer) {
            this.$refs.logContainer.scrollTop = this.$refs.logContainer.scrollHeight
          }
        })
      },
      deep: true
    }
  },
  methods: {
    getPokemonImage(id) {
      return `/images/pokemon_${id}.png`
    },
    
    getPokemonIdByName(name) {
      const pokemon = this.pokemonList.find(p => p.name === name)
      return pokemon ? pokemon.id : 1
    },
    
    handleImageError(e) {
      // 如果图片加载失败，使用占位图
      e.target.src = 'https://raw.githubusercontent.com/PokeAPI/sprites/master/sprites/pokemon/other/official-artwork/25.png'
    },
    
    parseSkillName(skill) {
      if (!skill) return '未知招式'
      const parts = skill.split('|')
      return parts[0] || '未知'
    },
    
    parseSkillDetail(skill) {
      if (!skill) return ''
      const parts = skill.split('|')
      if (parts.length >= 2) {
        const power = parts[1].split(':')[1]
        const pp = parts[3].split(':')[1]
        return `威力:${power} | ${pp}`
      }
      return ''
    },
    
    getLogClass(msg) {
      if (msg.includes('胜利')) return 'log-victory'
      if (msg.includes('伤害')) return 'log-damage'
      if (msg.includes('开始')) return 'log-start'
      return 'log-normal'
    },
    
    formatTime(idx) {
      const now = new Date()
      return `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}`
    },
    
    clearLog() {
      this.battleLog = ['战斗日志已清空']
    },
    
    async initGame() {
      try {
        const response = await axios.post('http://localhost:3000/api/init', {
          pid: this.selectedPid
        })
        const data = this.parseResponse(response.data)
        
        this.player = {
          name: data.PNAME,
          maxHp: parseInt(data.PHP),
          currentHp: parseInt(data.PHP)
        }
        this.enemy = {
          name: data.EMYNAME,
          maxHp: parseInt(data.EMYHP),
          currentHp: parseInt(data.EMYHP)
        }
        this.skills = [
          data.SKILL1, data.SKILL2, data.SKILL3, data.SKILL4
        ]
        this.battleLog = [`⚔️ 战斗开始！${this.player.name} VS ${this.enemy.name}`]
        this.gameStarted = true
        this.gameOver = false
      } catch (error) {
        console.error('初始化失败', error)
        this.battleLog.push('❌ 初始化失败，请检查后端服务')
      }
    },
    
    async useSkill(skillIndex) {
      if (this.gameOver) return
      
      this.isAttacking = 'player'
      
      try {
        const response = await axios.post('http://localhost:3000/api/fight', {
          pid: this.selectedPid,
          skill: skillIndex,
          php: this.player.currentHp,
          ehp: this.enemy.currentHp
        })
        const data = this.parseResponse(response.data)
        
        // 显示伤害效果
        this.battleLog.push(`✨ ${this.player.name} 使用 ${data.PATTACK}，造成 ${data.EMY_DMG} 伤害！`)
        
        setTimeout(() => {
          this.isAttacking = 'enemy'
          setTimeout(() => {
            this.battleLog.push(`💢 ${this.enemy.name} 使用 ${data.EMYATTACK}，造成 ${data.P_DMG} 伤害！`)
            this.isAttacking = null
          }, 500)
        }, 500)
        
        // 更新血量
        this.player.currentHp = parseInt(data.NEW_PHP)
        this.enemy.currentHp = parseInt(data.NEW_EMYHP)
        
        // 检查战斗结束
        if (parseInt(data.OVER) === 1) {
          setTimeout(() => {
            this.gameOver = true
            this.gameResult = data.WIN
            this.battleLog.push(`🏁 战斗结束！${data.WIN}`)
            this.isAttacking = null
          }, 1000)
        }
      } catch (error) {
        console.error('战斗失败', error)
        this.battleLog.push('⚠️ 战斗请求失败，请重试')
        this.isAttacking = null
      }
    },
    
    parseResponse(text) {
      const result = {}
      const lines = text.split(/\r?\n/)
      for (let line of lines) {
        if (line.trim()) {
          const idx = line.indexOf('=')
          if (idx === -1) continue
          const key = line.substring(0, idx).trim()
          const value = line.substring(idx + 1).trim()
          result[key] = value
        }
      }
      return result
    },
    
    resetGame() {
      this.gameStarted = false
      this.gameOver = false
      this.battleLog = []
      this.isAttacking = null
    }
  }
}
</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

.pokemon-battle {
  min-height: 100vh;
  background: url('/images/background.jpg') center/cover no-repeat fixed;
  position: relative;
  overflow-x: hidden;
}

.bg-decoration {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><circle cx="10" cy="10" r="2" fill="white" opacity="0.1"/><circle cx="90" cy="20" r="3" fill="white" opacity="0.1"/><circle cx="50" cy="80" r="2" fill="white" opacity="0.1"/></svg>');
  pointer-events: none;
}

/* 选择界面样式 */
.selection-screen {
  display: flex;
  justify-content: center;
  align-items: center;
  min-height: 100vh;
  padding: 20px;
}

.selection-card {
  background: rgba(255, 255, 255, 0.95);
  border-radius: 30px;
  padding: 40px;
  max-width: 1200px;
  width: 100%;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(10px);
}

.selection-header {
  text-align: center;
  margin-bottom: 40px;
}

.pokeball-icon {
  width: 60px;
  height: 60px;
  margin-bottom: 20px;
}

.selection-header h1 {
  font-size: 2.5em;
  color: #333;
  text-shadow: 2px 2px 4px rgba(0,0,0,0.1);
}

.pokemon-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
  gap: 20px;
  margin-bottom: 40px;
}

.pokemon-card {
  background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
  border-radius: 15px;
  padding: 15px;
  text-align: center;
  cursor: pointer;
  transition: all 0.3s ease;
  border: 3px solid transparent;
}

.pokemon-card:hover {
  transform: translateY(-5px);
  box-shadow: 0 10px 25px rgba(0,0,0,0.2);
}

.pokemon-card.selected {
  border-color: #ff6b6b;
  background: linear-gradient(135deg, #ffe6e6 0%, #ffcccc 100%);
  transform: scale(1.05);
}

.pokemon-image-wrapper {
  width: 80px;
  height: 80px;
  margin: 0 auto 10px;
}

.pokemon-selection-img {
  width: 100%;
  height: 100%;
  object-fit: contain;
}

.pokemon-name {
  font-weight: bold;
  font-size: 1.1em;
  color: #333;
}

.pokemon-id {
  font-size: 0.8em;
  color: #666;
  margin-top: 5px;
}

.start-btn {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  border: none;
  padding: 15px 40px;
  font-size: 1.2em;
  border-radius: 50px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 10px;
  margin: 0 auto;
  transition: all 0.3s ease;
}

.start-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 10px 20px rgba(0,0,0,0.2);
}

/* 对战界面样式 */
.battle-screen {
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  padding: 20px;
}

.battle-area {
  flex: 1;
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 40px;
  padding: 40px;
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-radius: 30px;
  margin-bottom: 20px;
}

.pokemon-side {
  flex: 1;
  display: flex;
  gap: 20px;
  background: rgba(255, 255, 255, 0.9);
  border-radius: 20px;
  padding: 20px;
  box-shadow: 0 10px 30px rgba(0,0,0,0.2);
}

.enemy-side {
  flex-direction: row;
}

.player-side {
  flex-direction: row-reverse;
}

.pokemon-info {
  flex: 1;
}

.pokemon-name-tag {
  display: flex;
  justify-content: space-between;
  align-items: baseline;
  margin-bottom: 10px;
}

.pokemon-name {
  font-size: 1.3em;
  font-weight: bold;
  color: #333;
}

.pokemon-level {
  font-size: 0.9em;
  color: #666;
  background: #f0f0f0;
  padding: 2px 8px;
  border-radius: 10px;
}

.hp-container {
  margin-top: 10px;
}

.hp-label {
  display: flex;
  justify-content: space-between;
  font-size: 0.9em;
  margin-bottom: 5px;
  color: #666;
}

.hp-bar-bg {
  background: #e0e0e0;
  height: 15px;
  border-radius: 10px;
  overflow: hidden;
  position: relative;
}

.hp-bar-fill {
  height: 100%;
  transition: width 0.5s ease;
  position: relative;
  overflow: hidden;
}

.player-hp {
  background: linear-gradient(90deg, #ff6b6b, #ff4757);
}

.enemy-hp {
  background: linear-gradient(90deg, #ffa502, #ff6348);
}

.hp-bar-glow {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(90deg, transparent, rgba(255,255,255,0.3), transparent);
  animation: shimmer 2s infinite;
}

@keyframes shimmer {
  0% { transform: translateX(-100%); }
  100% { transform: translateX(100%); }
}

.pokemon-sprite {
  position: relative;
  width: 150px;
  height: 150px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.battle-sprite {
  width: 100%;
  height: 100%;
  object-fit: contain;
  transition: all 0.3s ease;
}

.enemy-sprite {
  animation: float 3s ease-in-out infinite;
}

.player-sprite {
  animation: float 3s ease-in-out infinite reverse;
}

.low-hp {
  filter: hue-rotate(15deg) brightness(0.8);
  animation: shake 0.5s ease-in-out;
}

@keyframes float {
  0%, 100% { transform: translateY(0px); }
  50% { transform: translateY(-10px); }
}

@keyframes shake {
  0%, 100% { transform: translateX(0); }
  25% { transform: translateX(-5px); }
  75% { transform: translateX(5px); }
}

.sprite-effects {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 3em;
  animation: effectAnim 0.5s ease-out;
}

@keyframes effectAnim {
  0% { transform: scale(0); opacity: 1; }
  100% { transform: scale(2); opacity: 0; }
}

.vs-divider {
  display: flex;
  align-items: center;
  justify-content: center;
}

.vs-circle {
  width: 80px;
  height: 80px;
  background: linear-gradient(135deg, #ff6b6b, #ff4757);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: bold;
  font-size: 1.5em;
  color: white;
  box-shadow: 0 5px 15px rgba(0,0,0,0.3);
  animation: pulse 2s infinite;
}

@keyframes pulse {
  0%, 100% { transform: scale(1); }
  50% { transform: scale(1.1); }
}

.action-area {
  margin-bottom: 20px;
}

.skills-panel {
  background: rgba(255, 255, 255, 0.95);
  border-radius: 20px;
  padding: 20px;
  backdrop-filter: blur(10px);
}

.skills-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 15px;
  padding-bottom: 10px;
  border-bottom: 2px solid #e0e0e0;
  font-weight: bold;
  color: #333;
}

.turn-indicator {
  background: linear-gradient(135deg, #667eea, #764ba2);
  color: white;
  padding: 5px 15px;
  border-radius: 20px;
  font-size: 0.9em;
}

.skills-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 15px;
}

.skill-btn {
  background: linear-gradient(135deg, #f5f7fa, #c3cfe2);
  border: none;
  padding: 15px;
  border-radius: 15px;
  cursor: pointer;
  transition: all 0.3s ease;
  text-align: left;
}

.skill-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 5px 15px rgba(0,0,0,0.2);
  background: linear-gradient(135deg, #e0e7ff, #b8c5e0);
}

.skill-name {
  font-weight: bold;
  font-size: 1.1em;
  color: #333;
  margin-bottom: 5px;
}

.skill-detail {
  font-size: 0.8em;
  color: #666;
}

/* 战斗日志样式 */
.battle-log {
  background: rgba(0, 0, 0, 0.8);
  backdrop-filter: blur(10px);
  border-radius: 20px;
  overflow: hidden;
  margin-top: 20px;
}

.log-header {
  background: linear-gradient(135deg, #667eea, #764ba2);
  padding: 12px 20px;
  display: flex;
  justify-content: space-between;
  align-items: center;
  color: white;
  font-weight: bold;
}

.clear-log-btn {
  background: rgba(255,255,255,0.2);
  border: none;
  color: white;
  padding: 5px 15px;
  border-radius: 20px;
  cursor: pointer;
  transition: all 0.3s ease;
}

.clear-log-btn:hover {
  background: rgba(255,255,255,0.3);
}

.log-content {
  height: 200px;
  overflow-y: auto;
  padding: 15px;
}

.log-entry {
  padding: 8px 12px;
  margin-bottom: 8px;
  border-radius: 10px;
  font-size: 0.9em;
  display: flex;
  gap: 10px;
  animation: slideIn 0.3s ease-out;
}

@keyframes slideIn {
  from {
    opacity: 0;
    transform: translateX(-20px);
  }
  to {
    opacity: 1;
    transform: translateX(0);
  }
}

.log-time {
  color: #888;
  font-family: monospace;
  font-size: 0.8em;
}

.log-message {
  flex: 1;
}

.log-damage {
  background: rgba(255, 107, 107, 0.2);
  border-left: 3px solid #ff6b6b;
}

.log-victory {
  background: rgba(255, 215, 0, 0.2);
  border-left: 3px solid gold;
  font-weight: bold;
}

.log-start {
  background: rgba(102, 126, 234, 0.2);
  border-left: 3px solid #667eea;
}

.log-normal {
  background: rgba(255, 255, 255, 0.1);
}

/* 游戏结束界面 */
.game-over-panel {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0,0,0,0.8);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
  backdrop-filter: blur(5px);
}

.result-card {
  background: white;
  border-radius: 30px;
  padding: 40px;
  text-align: center;
  animation: bounceIn 0.6s ease-out;
}

@keyframes bounceIn {
  0% {
    transform: scale(0);
    opacity: 0;
  }
  50% {
    transform: scale(1.1);
  }
  100% {
    transform: scale(1);
    opacity: 1;
  }
}

.result-icon {
  font-size: 4em;
  margin-bottom: 20px;
}

.restart-btn {
  background: linear-gradient(135deg, #667eea, #764ba2);
  color: white;
  border: none;
  padding: 12px 30px;
  font-size: 1.1em;
  border-radius: 50px;
  cursor: pointer;
  margin-top: 20px;
  transition: all 0.3s ease;
}

.restart-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 10px 20px rgba(0,0,0,0.2);
}

/* 响应式设计 */
@media (max-width: 768px) {
  .battle-area {
    flex-direction: column;
    gap: 20px;
  }
  
  .pokemon-side {
    width: 100%;
  }
  
  .skills-grid {
    grid-template-columns: 1fr;
  }
  
  .pokemon-grid {
    grid-template-columns: repeat(auto-fill, minmax(100px, 1fr));
  }
}
</style>