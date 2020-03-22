Feature:
  Scenario Outline:
    Given user has a valid card;
    And account balance is <40>;
    When they insert the card;
    And withdraw <17>;
    Then the ATM should return <17>;
    And the balance should be <23>
