# TOWER_DEFENSE
Add inclue json vào general

Game.cpp
Update Game::Game()
- Thêm Menu, Background music, exit game
Update  Game::run(), Game::UpdatePlay(), Game::UpdateTower(), Game::UpdateAxe(), Game::CheckForDeletionRequest()
- Kiểm tra paused khi trong gameplay
- Thêm sound cho background, đặt tower, quăng rìu, enemies died
Update Game::Draw()
- Vẽ menu khi paused game
Update Game::HandleInput()
-  Xử lý khi người chơi pause game
-  Tạm dừng/ Phát lại nhạc nền
Update Game::HandleGameInput()
- Thêm phía Esc để quay về menu
- Xử lý nếu trong game ấn Esc thì quay về pause menu
Update Game::StartGame()
- Xử lý save game khi chọn existing profile
Thêm Game::ReturnToMenu()
- Để quay về Main menu
Thêm Game::ExitGame()
- Để thoát game
Thêm Game::ResetGameState()
- Reset game về trạng thái ban đầu nếu create a new profile hoặc play a guest.
Thêm Game::SetMusicVolume(), Game::SetSoundVolume()
- Điều chỉnh to nhỏ cho background music và sound âm thanh game

