// Copyright Breachpoint. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class ULobbyUI;

/**
 * Player controller used in the lobby level.
 * Creates and manages the lobby UI widget and handles the ready-up state.
 */
UCLASS()
class BREACHPOINT_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALobbyPlayerController();

	//~ Begin APlayerController Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End APlayerController Interface

	/** The widget class to spawn for the lobby UI (assign a UMG Blueprint subclass) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby|UI")
	TSubclassOf<ULobbyUI> LobbyUIClass;

	/** Returns the active lobby UI widget, if any */
	UFUNCTION(BlueprintCallable, Category = "Lobby|UI")
	ULobbyUI* GetLobbyUI() const { return LobbyUIWidget; }

	/** Whether this player has readied up */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsReady() const { return bIsReady; }

	/** Toggles the ready state. Callable from UI or input. */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ToggleReady();

	/** Sets the ready state explicitly */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bNewReady);

	/** Server RPC - notify the server of ready-state change */
	UFUNCTION(Server, Reliable)
	void ServerSetReady(bool bNewReady);

protected:
	/** Creates and displays the lobby UI widget */
	void CreateLobbyUI();

	/** Removes the lobby UI widget */
	void DestroyLobbyUI();

private:
	/** The live lobby UI widget instance */
	UPROPERTY()
	TObjectPtr<ULobbyUI> LobbyUIWidget = nullptr;

	/** Current ready state */
	UPROPERTY(Replicated)
	bool bIsReady = false;

	//~ Begin AActor Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor Interface
};
