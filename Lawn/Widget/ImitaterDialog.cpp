#include "../Plant.h"
#include "GameButton.h"
#include "../SeedPacket.h"
#include "../../LawnApp.h"
#include "ImitaterDialog.h"
#include "SeedChooserScreen.h"
#include "../ToolTipWidget.h"
#include "../../GameConstants.h"
#include "../../SexyAppFramework/WidgetManager.h"

ImitaterDialog::ImitaterDialog() : 
	LawnDialog(gLawnApp, Dialogs::DIALOG_IMITATER, true, _S("[CHOOSE_SEED_TO_COPY]"), _S(""), _S("[DIALOG_BUTTON_OK]"), Dialog::BUTTONS_YES_NO)
{
	CalcSize(IMITATER_DIALOG_WIDTH - mWidth, IMITATER_DIALOG_HEIGHT - mHeight);
	mToolTip = new ToolTipWidget();
	mClip = false;
	mToolTipSeed = SeedType::SEED_NONE;
	mLawnYesButton->mBtnNoDraw = true;
	mLawnYesButton->mMouseVisible = false;
	mLawnNoButton->mBtnNoDraw = true;
	mLawnNoButton->mMouseVisible = false;
}

ImitaterDialog::~ImitaterDialog()
{
	delete mToolTip;
}

SeedType ImitaterDialog::SeedHitTest(int x, int y)
{
	for (SeedType aSeedType = (SeedType)0; aSeedType < SeedType::SEED_GATLINGPEA; aSeedType = (SeedType)(aSeedType + 1))
	{
		if (mApp->SeedTypeAvailable(aSeedType))
		{
			int aSeedX, aSeedY;
			GetSeedPosition(aSeedType, aSeedX, aSeedY);
			if (Rect(aSeedX, aSeedY, SEED_PACKET_WIDTH, SEED_PACKET_HEIGHT).Contains(x, y))
			{
				return aSeedType;
			}
		}
	}
	return SeedType::SEED_NONE;
}

void ImitaterDialog::UpdateCursor()
{
	SeedType aSeedType = SeedHitTest(mApp->mWidgetManager->mLastMouseX - mX, mApp->mWidgetManager->mLastMouseY - mY);
	if (aSeedType != SeedType::SEED_NONE && !mApp->mSeedChooserScreen->SeedNotAllowedToPick(aSeedType))
	{
		mApp->SetCursor(CURSOR_HAND);
	}
	else
	{
		mApp->SetCursor(CURSOR_POINTER);
	}
}

void ImitaterDialog::Update()
{
	LawnDialog::Update();
	ShowToolTip();
	UpdateCursor();
}

void ImitaterDialog::GetSeedPosition(int theIndex, int& x, int& y)
{
	x = (theIndex % 8) * (SEED_PACKET_WIDTH + 1) + mWidth / 2 - 210;
	y = (theIndex / 8) * (SEED_PACKET_HEIGHT + 1) + 112;
}

void ImitaterDialog::Draw(Graphics* g)
{
	LawnDialog::Draw(g);
	g->SetLinearBlend(true);
	for (SeedType aSeedType = (SeedType)0; aSeedType < SeedType::SEED_GATLINGPEA; aSeedType = (SeedType)(aSeedType + 1))
	{
		if (mApp->SeedTypeAvailable(aSeedType))
		{
			int aSeedX, aSeedY;
			GetSeedPosition(aSeedType, aSeedX, aSeedY);
			bool aGrayed = mApp->mSeedChooserScreen->SeedNotAllowedToPick(aSeedType) || mApp->mSeedChooserScreen->SeedNotRecommendedToPick(aSeedType);
			DrawSeedPacket(g, aSeedX, aSeedY, SeedType::SEED_IMITATER, aSeedType, 0, aGrayed ? 115 : 255, true, false);
		}
	}
	mToolTip->Draw(g);
}

void ImitaterDialog::ShowToolTip()
{
	if (!mApp->mWidgetManager->mMouseIn || !mApp->mActive)
	{
		RemoveToolTip();
		return;
	}
	
	SeedType aSeedType = SeedHitTest(mApp->mWidgetManager->mLastMouseX - mX, mApp->mWidgetManager->mLastMouseY - mY);
	if (aSeedType == SeedType::SEED_NONE)
	{
		RemoveToolTip();
	}
	else if (aSeedType != mToolTipSeed)
	{
		RemoveToolTip();
		uint aRecFlags = mApp->mSeedChooserScreen->SeedNotRecommendedToPick(aSeedType);
		if (mApp->mSeedChooserScreen->SeedNotAllowedToPick(aSeedType))  
		{
			mToolTip->SetWarningText(_S("[NOT_ALLOWED_ON_THIS_LEVEL]"));
		}
		else if (aRecFlags)  
		{
			if (TestBit(aRecFlags, NotRecommend::NOT_RECOMMENDED_NOCTURNAL))
			{
				mToolTip->SetWarningText(_S("[NOCTURNAL_WARNING]"));
			}
			else
			{
				mToolTip->SetWarningText(_S("[NOT_RECOMMENDED_FOR_LEVEL]"));
			}
		}
		else
		{
			mToolTip->SetWarningText(_S(""));
		}
		mToolTip->SetTitle(Plant::GetNameString(SeedType::SEED_IMITATER, aSeedType));
		mToolTip->SetLabel(Plant::GetToolTip(aSeedType));

		int aSeedX, aSeedY;
		GetSeedPosition(aSeedType, aSeedX, aSeedY);
		mToolTip->mX = (SEED_PACKET_WIDTH - mToolTip->mWidth) / 2 + aSeedX;
		mToolTip->mY = SEED_PACKET_HEIGHT + aSeedY;
		mToolTip->mVisible = true;
		mToolTipSeed = aSeedType;
	}
}

void ImitaterDialog::RemoveToolTip()
{
	mToolTip->mVisible = false;
	mToolTipSeed = SeedType::SEED_NONE;
}

void ImitaterDialog::MouseDown(int x, int y, int theClickCount)
{
	SeedType aSeedType = SeedHitTest(x, y);
	if (aSeedType != SeedType::SEED_NONE)
	{
		SeedChooserScreen* aSeedChooser = mApp->mSeedChooserScreen;
		if (!aSeedChooser->SeedNotAllowedToPick(aSeedType))
		{
			ChosenSeed& aImitater = aSeedChooser->mChosenSeeds[SeedType::SEED_IMITATER];
			aImitater.mSeedState = ChosenSeedState::SEED_IN_CHOOSER;
			aImitater.mImitaterType = aSeedType;
			aImitater.mX = aSeedChooser->mImitaterButton->mX;
			aImitater.mY = aSeedChooser->mImitaterButton->mY;
			aSeedChooser->ClickedSeedInChooser(aImitater);
			//aSeedChooser->UpdateImitaterButton();
			mApp->KillDialog(mId);
		}
	}
	else
	{
		Widget::MouseDown(x, y, theClickCount);
	}
}
